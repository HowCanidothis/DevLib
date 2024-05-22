#ifndef PROPERTIESTOOLVIEW_H
#define PROPERTIESTOOLVIEW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <UnitsModule/internal.hpp>
#include <WidgetsModule/internal.hpp>

struct LineData;
class QPushButton;
class PropertiesToolView;
class WidgetPropertiesToolViewWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetPropertiesToolViewWrapper(class PropertiesToolView* folder);
    LocalPropertyBoolCommutator& ContentVisibilityCommutator() const;
    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetPropertiesToolViewWrapper, PropertiesToolView)
};

template <typename T>
struct TPropertiesToolWrapper;
struct PropertiesToolViewWrapper {
    WidgetAbstractButtonWrapper HeaderWrapper;
    WidgetPropertiesToolViewWrapper FolderWrapper;
    LocalPropertyBool& WidgetVisibility() const { return HeaderWrapper.WidgetVisibility(); }
    PropertiesToolView* GetView() const { return FolderWrapper.GetWidget(); }
    CommonDispatcher<const LineData&>& OnPropertyAdded() const;

    LocalPropertyBoolCommutator& ContentVisibilityCommutator() const;

    PropertiesToolViewWrapper(QAbstractButton* header = nullptr, PropertiesToolView* body = nullptr)
        : HeaderWrapper(header)
        , FolderWrapper(body)
    {}

    bool operator==(const PropertiesToolViewWrapper& another) const { return GetView() == another.GetView(); }

    template <typename T>
    struct TPropertiesToolWrapper<T> CreateObjectWrapper(T* object, bool visibleByDefault = false);
};

class PropertiesToolFolderView : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QSize buttonOffset MEMBER m_buttonOffset)
    using Super = QFrame;
public:
    struct BindingRules
    {
        QAbstractButton* ToolButton;
        QWidget* Widget;
        FAction Binding;
        FAction ReleaseBind;
        FAction Destroy;

        BindingRules(QWidget* widget, const FAction& bind = nullptr, const FAction& release = nullptr)
            : ToolButton(nullptr)
            , Widget(widget)
            , Binding(bind)
            , ReleaseBind(release)
        {}
    };

    PropertiesToolFolderView(QWidget* parent = nullptr);

    PropertiesToolViewWrapper BeginFolder(const Name& folderName, const FTranslationHandler& title, int index = -1);
    WidgetAbstractButtonWrapper BeginFolder(const Name& folderName, const FTranslationHandler& title, const BindingRules& widget, int index = -1);
    void Clear();

    bool DestroyFolder(const Name& folderName);
    int Count();
    DispatcherConnection AddDeleteButton(const WidgetAbstractButtonWrapper& folder, const FAction& onClicked);
    void AddWidget(QWidget* w, int index = -1);
    void AddLayout(QLayout* l, int index = -1);

    void Bind();
    void Release();

    LocalPropertyBool ForceDisabled;

private:
    class QVBoxLayout* m_layout;
    QHash<Name, BindingRules> m_widgets;
    DispatcherConnectionsSafe m_connections;
    QPushButton* m_deleteButton;
    QAbstractButton* m_deleteButtonOverFolder;
    QSize m_buttonOffset;
};
Q_DECLARE_METATYPE(SharedPointer<LocalPropertyBoolCommutator>)

struct LineData {
    LineData();
    LineData(const Name& id, QWidget* w, ElidedLabel* lb = nullptr);

    ElidedLabel* Label;
    QWidget* Editor;
    WidgetWrapper EditorWrapper;
    WidgetElidedLabelWrapper LabelWrapper;
    LocalPropertyBool& WidgetEnablity() const { return EditorWrapper.WidgetEnablity(); }
    LocalPropertyBool& WidgetVisibility() const { return EditorWrapper.WidgetVisibility(); }
//    LocalPropertySequentialEnum<HighLightEnum>& WidgetHighlighted() const { return EditorWrapper.WidgetHighlighted(); }
    LocalPropertyBoolCommutator& ContentVisibilityCommutator() const;

    const Name& GetId() const { return m_id; }
    template<typename Widget>
    Widget* GetEditor() const { return reinterpret_cast<Widget*>(Editor); }
    template<typename Wrapper>
    const Wrapper& GetEditorWrapper() const { return reinterpret_cast<const Wrapper&>(EditorWrapper); }

    operator qint64() const { return m_id; }
private:
    Name m_id;
};

class PropertiesToolView : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
public:
    PropertiesToolView(QWidget* parent = nullptr);

    Name Key;
    LineData AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyString* ()>& propertyGetter)
    {
        auto* cb = new QComboBox;
        cb->setEditable(true);

        return addProperty(propertyName, title, cb, [this, propertyGetter](QWidget* w){
            auto* property = propertyGetter();
            if(property == nullptr) {
                return;
            }
            auto* comboBox = reinterpret_cast<QComboBox*>(w);
            m_connectors.AddConnector<LocalPropertiesLineEditConnector>(property, comboBox->lineEdit());
        });
    }

    template<class T>
    LineData AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertySequentialEnum<T>* ()>& propertyGetter)
    {
        auto* cb = new QComboBox;
        WidgetComboboxWrapper(cb).SetEnum<T>();

        return addProperty(propertyName, title, cb, [this, propertyGetter](QWidget* w){
            auto* property = propertyGetter();
            if(property == nullptr) {
                return;
            }
            auto* comboBox = reinterpret_cast<QComboBox*>(w);
            m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(property, comboBox);
        });
    }

    template<class Property>
    LineData AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<Property* ()>& propertyGetter, const std::function<ModelsStandardListModelPtr ()>& modelGetter)
    {
        auto* cb = new QComboBox;
        auto* filterModel = new ViewModelsFilterModelBase(cb);
        auto* viewModel = new ViewModelsStandardListModel(filterModel);
        filterModel->setSourceModel(viewModel);
        filterModel->setDynamicSortFilter(true);
        filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        cb->setModel(filterModel);

        return addProperty(propertyName, title, cb, [this, propertyGetter, modelGetter, viewModel, filterModel](QWidget* w){
            auto* property = propertyGetter();
            if(property == nullptr) {
                return;
            }
            const auto& model = modelGetter();
            viewModel->SetData(model);
            auto* comboBox = reinterpret_cast<QComboBox*>(w);

            m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(property, comboBox, IdRole);
        });
    }

    LineData AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyBool* ()>& propertyGetter)
    {
        return addProperty(propertyName, title, new QCheckBox, [this, propertyGetter](QWidget* w){
            auto* property = propertyGetter();
            if(property == nullptr) {
                return;
            }
            auto* checkBox = reinterpret_cast<QCheckBox*>(w);
            m_connectors.AddConnector<LocalPropertiesCheckBoxConnector>(property, checkBox);
        });
    }

    template<class Property>
    LineData AddProperty(const Name& propertyName, const FTranslationHandler& label, const std::function<Property* ()>& propertyGetter);

    template<class Property>
    LineData AddProperty(const Name& propertyName, const Measurement* measurement, const FTranslationHandler& title, const std::function<Property* ()>& propertyGetter)
    {
        auto* spinBox = new WidgetsDoubleSpinBoxWithCustomDisplay();
        spinBox->setButtonSymbols(QDoubleSpinBox::NoButtons);

        return addProperty(propertyName, title, spinBox, [this, propertyName, propertyGetter, measurement, title](QWidget* w){
            auto* property = propertyGetter();
            if(property == nullptr) {
                return;
            }
            auto* spinBox = reinterpret_cast<WidgetsDoubleSpinBoxWithCustomDisplay*>(w);
            ///TODO: kostil with label
            m_connectors.AddConnector(measurement, property, spinBox, FindRow(propertyName).Label, title, {});
        });
    }

    LineData AddDoubleProperty(const Name& propertyName, const Measurement* measurement, const FTranslationHandler& title, const std::function<LocalPropertyDoubleOptional* ()>& propertyGetter)
    {
        return AddProperty<LocalPropertyDoubleOptional>(propertyName, measurement, title, propertyGetter);
    }

    const LineData& FindRow(const Name& propertyName) const;

    void BeginGroup(const FTranslationHandler& header);

    LineData AddData(const Name& id, QWidget* widget, const FTranslationHandler& title = nullptr, Qt::Orientation orientation = Qt::Horizontal);

    void Bind();
    void ClearBindings();

    LocalPropertyBool ForceDisabled;

    mutable CommonDispatcher<const LineData&> OnPropertyAdded;

private:
    LineData addProperty(const Name& propertyName, const FTranslationHandler& title, QWidget* widget, const std::function<void(QWidget*)>& binding);

private:
    friend class PropertiesToolFolderView;
    struct Binding
    {
        LineData Data;
        std::function<void (QWidget* w)> BindingHandler;
    };

    class QGridLayout* m_layout;
    QHash<Name, Binding> m_bindings;
    MeasurementWidgetConnectors m_connectors;
    DispatcherConnectionsSafe m_connections;
};

template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyColor>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyColor* ()>& propertyGetter)
{
    auto* cb = new WidgetsColorPicker();

    return addProperty(propertyName, title, cb, [this, propertyGetter, cb](QWidget*){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(property, cb);
    });
}

template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyString>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyString* ()>& propertyGetter)
{
    return addProperty(propertyName, title, new QLineEdit, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* lineEdit = reinterpret_cast<QLineEdit*>(w);
        m_connectors.AddConnector<LocalPropertiesLineEditConnector>(property, lineEdit);
    });
}

template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyInt>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyInt* ()>& propertyGetter)
{
    auto* spinBox = new WidgetsSpinBoxWithCustomDisplay();
    spinBox->setButtonSymbols(QSpinBox::NoButtons);

    return addProperty(propertyName, title, spinBox, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* spinBox = reinterpret_cast<WidgetsSpinBoxWithCustomDisplay*>(w);
        m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(property, spinBox);
    });
}

template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyIntOptional>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyIntOptional* ()>& propertyGetter)
{
    auto* spinBox = new WidgetsSpinBoxWithCustomDisplay();
    spinBox->setButtonSymbols(QSpinBox::NoButtons);

    return addProperty(propertyName, title, spinBox, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* spinBox = reinterpret_cast<WidgetsSpinBoxWithCustomDisplay*>(w);
        m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(property, spinBox);
    });
}

template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyDouble>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyDouble* ()>& propertyGetter)
{
    auto* spinBox = new WidgetsDoubleSpinBoxWithCustomDisplay();
    spinBox->setButtonSymbols(QDoubleSpinBox::NoButtons);

    return addProperty(propertyName, title, spinBox, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* spinBox = reinterpret_cast<WidgetsDoubleSpinBoxWithCustomDisplay*>(w);
        m_connectors.AddConnector<LocalPropertiesDoubleSpinBoxConnector>(property, spinBox);
    });
}

template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyDoubleOptional>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyDoubleOptional* ()>& propertyGetter)
{
    auto* spinBox = new WidgetsDoubleSpinBoxWithCustomDisplay();
    spinBox->setButtonSymbols(QDoubleSpinBox::NoButtons);

    return addProperty(propertyName, title, spinBox, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* spinBox = reinterpret_cast<WidgetsDoubleSpinBoxWithCustomDisplay*>(w);
        m_connectors.AddConnector<LocalPropertiesDoubleSpinBoxConnector>(property, spinBox);
    });
}


template<>
inline LineData PropertiesToolView::AddProperty<LocalPropertyBool>(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyBool* ()>& propertyGetter)
{
    return addProperty(propertyName, title, new QCheckBox(), [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* spinBox = reinterpret_cast<QCheckBox*>(w);
        m_connectors.AddConnector<LocalPropertiesCheckBoxConnector>(property, spinBox);
    });
}

template <typename T>
struct TPropertiesToolWrapper {
    TPropertiesToolWrapper(T* object, PropertiesToolView* folder, bool visibleByDefault = false)
        : m_folder(folder)
        , m_object(object)
    {
        if(visibleByDefault) {
            OnPropertyAdded.Connect(CDL, [](const LineData& ld) {
                ld.EditorWrapper.WidgetVisibility() = true;
            });
        }
    }

    void BeginGroup(const FTranslationHandler& header)
    {
        m_folder->BeginGroup(header);
    }

    LineData AddWidget(const Name& id, QWidget* widget, const FTranslationHandler& title = nullptr, Qt::Orientation orientation = Qt::Horizontal){
        return Register(m_folder->AddData(id, widget, title, orientation));
    }

    template<class Property>
    LineData AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<Property& (T*)>& propertyGetter, const std::function<ModelsStandardListModelPtr ()>& modelGetter)
    {
        auto* property = &propertyGetter(m_object);
        return Register(m_folder->AddProperty<Property>(propertyName, title, [property]{ return property; }, modelGetter));
    }

    template<typename Property>
    LineData AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<Property& (T*)>& propertyGetter){
        auto* property = &propertyGetter(m_object);
        return Register(m_folder->AddProperty<Property>(propertyName, title, [property]{ return property; }));
    }
    LineData AddBoolProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyBool& (T*)>& propertyGetter){
        return AddProperty<LocalPropertyBool>(propertyName, title, propertyGetter);
    }
    LineData AddStringProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyString& (T*)>& propertyGetter){
        return AddProperty<LocalPropertyString>(propertyName, title, propertyGetter);
    }
    LineData AddColorProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyColor& (T*)>& propertyGetter) {
        return AddProperty<LocalPropertyColor>(propertyName, title, propertyGetter);
    }
    template<class Enum>
    LineData AddEnumProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertySequentialEnum<Enum>& (T*)>& propertyGetter){
        auto* property = &propertyGetter(m_object);
        return Register(m_folder->AddProperty<Enum>(propertyName, title, [property]{ return property; }));
    }
    LineData AddIntProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyIntOptional&(T*)>& propertyGetter){
        return AddProperty<LocalPropertyIntOptional>(propertyName, title, propertyGetter);
    }
    LineData AddIntProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyInt&(T*)>& propertyGetter){
        return AddProperty<LocalPropertyInt>(propertyName, title, propertyGetter);
    }
    LineData AddDoubleProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyDoubleOptional&(T*)>& propertyGetter){
        return AddProperty<LocalPropertyDoubleOptional>(propertyName, title, propertyGetter);
    }
    LineData AddDoubleProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyDouble&(T*)>& propertyGetter){
        return AddProperty<LocalPropertyDouble>(propertyName, title, propertyGetter);
    }
    LineData AddDoubleProperty(const Name& propertyName, const Measurement* measurement, const FTranslationHandler& title, const std::function<LocalPropertyDoubleOptional&(T*)>& propertyGetter){
        auto property = &propertyGetter(m_object);
        FTranslationHandler titleUnit;
        if(title().contains(MEASUREMENT_UN)){
            titleUnit = title;
        } else {
            titleUnit = TR(QString("%1 (%un)").arg(title()), title);
        }
        return Register(m_folder->AddDoubleProperty(propertyName, measurement, title, [property]{ return property; }));
    }
    LineData AddDoubleProperty(const Name& propertyName, const Measurement* measurement, const FTranslationHandler& title, const std::function<LocalPropertyDouble&(T*)>& propertyGetter){
        auto property = &propertyGetter(m_object);
        FTranslationHandler titleUnit;
        if(title().contains(MEASUREMENT_UN)){
            titleUnit = title;
        } else {
            titleUnit = TR(QString("%1 (%un)").arg(title()), title);
        }
        return Register(m_folder->AddProperty<LocalPropertyDouble>(propertyName, measurement, title, [property]{ return property; }));
    }
//    LineData AddDoubleProperty(const Name& propertyName, const Measurement* measurement, const FTranslationHandler& title, const std::function<StateParameterProperty<LocalPropertyDoubleOptional>&(T*)>& propertyGetter){
//        return AddDoubleProperty(propertyName, title, measurement, [propertyGetter](T* o) -> LocalPropertyDoubleOptional& { return propertyGetter(o).InputValue; });
//    }

    TPropertiesToolWrapper<T>& WrapVisible(const char* locationInfo, DispatcherConnectionsSafe& connections){
        return WrapVisible(locationInfo, &m_object->IsActive, connections);
    }

    TPropertiesToolWrapper<T>& WrapVisible(const char* locationInfo, LocalPropertyBool* active, DispatcherConnectionsSafe& connections){
        auto* pConnections = &connections;
        auto subscribe = [locationInfo, active, pConnections](const LineData& data){
            data.ContentVisibilityCommutator().ConnectFrom(locationInfo, *active).MakeSafe(*pConnections);
        };
        for(const auto& data : m_properties){
            subscribe(data);
        }
        OnPropertyAdded.Connect(locationInfo, subscribe).MakeSafe(connections);
        return *this;
    }

    CommonDispatcher<const LineData&> OnPropertyAdded;
private:
    LineData Register(const LineData& data){
        m_properties.insert(data);
        OnPropertyAdded(data);
        return data;
    }

    T* m_object;
    PropertiesToolView* m_folder;
    QSet<LineData> m_properties;
};

template<typename T>
struct TPropertiesToolWrapper<T> PropertiesToolViewWrapper::CreateObjectWrapper(T* object, bool visibleByDefault){
    return {object, GetView(), visibleByDefault};
}
inline CommonDispatcher<const LineData&>& PropertiesToolViewWrapper::OnPropertyAdded() const {
    return GetView()->OnPropertyAdded;
}
#endif // PROPERTIESTOOLVIEW_H
