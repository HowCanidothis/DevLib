#ifndef WIDGETHELPERS_H
#define WIDGETHELPERS_H

#include <QCompleter>
#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"
#include "WidgetsModule/Dialogs/widgetsdialog.h"

struct WidgetWrapperInjectedCommutatorData
{
    DispatchersCommutator Commutator;
    DispatcherConnectionsSafe Connections;

    WidgetWrapperInjectedCommutatorData(qint32 delay = 1000)
        : Commutator(delay)
    {}
};

class EventFilterObject : public QObject
{
    Q_OBJECT
    using Super = QObject;

    using FFilter = std::function<bool (QObject*, QEvent*)>;
    EventFilterObject(const FFilter& filter, QObject* parent);

    bool eventFilter(QObject* watched, QEvent* e) override;

protected:
    friend class ObjectWrapper;
    FFilter m_filter;
};

enum class HighLightEnum {
    None,
    Critical,
    Warning,
    First = None,
    Last = Warning,
};

Q_DECLARE_METATYPE(SharedPointer<WidgetWrapperInjectedCommutatorData>)

#define DECLARE_WIDGET_WRAPPER_ADD_CHECKED(WrapperType) \
    const WrapperType& SetChecked(bool checked) const { return setChecked<WrapperType>(checked); }
#define DECLARE_WIDGET_WRAPPER_ADD_TEXT(WrapperType) \
    template<typename ... Dispatchers> \
    const WrapperType& SetText(const FTranslationHandler& handler, Dispatchers&... dispatchers) const { return setText<WrapperType>(handler, dispatchers...); }
#define DECLARE_WIDGET_WRAPPER_ADD_CLICKED(WrapperType) \
    const WrapperType& SetOnClicked(const FAction& action) const { return setOnClicked<WrapperType>(action); }

#define DECLARE_WIDGET_WRAPPER_FUNCTIONS_BASE(WrapperType, type) \
    const WrapperType& Make(const std::function<void (const WrapperType&)>& handler) const { return make<WrapperType>(handler); } \
    type* GetWidget() const { return reinterpret_cast<type*>(m_object); } \
    type* operator->() const { return reinterpret_cast<type*>(m_object); } \
    operator type*() const { return reinterpret_cast<type*>(m_object); } \
    using expected_type = type;
#define DECLARE_WIDGET_WRAPPER_FUNCTIONS(WrapperType, type) \
    DECLARE_WIDGET_WRAPPER_FUNCTIONS_BASE(WrapperType, type) \
    const WrapperType& SetVisible(bool visible) const { return setVisible<WrapperType>(visible); } \
    const WrapperType& SetEnabled(bool enabled) const { return setEnabled<WrapperType>(enabled); } \
    const WrapperType& SetHighlighted(bool highlighted) const { return setHighlighted<WrapperType>(highlighted); } \
    template<typename ... Dispatchers> \
    const WrapperType& SetToolTip(const FTranslationHandler& handler, Dispatchers&... dispatchers) const { return setToolTip<WrapperType>(handler, dispatchers...); }

class ObjectWrapper
{
public:
    ObjectWrapper(QObject* object)
        : m_object(object)
    {}

    DispatcherConnectionsSafe& ObjectConnections() const;
    EventFilterObject* AddEventFilter(const std::function<bool (QObject*, QEvent*)>& filter) const;

    template<class T> const T& Cast() const { Q_ASSERT(qobject_cast<typename T::expected_type*>(m_object)); return *((const T*)this); }

    template<class T>
    SharedPointer<T> Injected(const char* propertyName, const std::function<T* ()>& creator = nullptr) const
    {
        SP<T> value = m_object->property(propertyName).value<SharedPointer<T>>();
        if(value == nullptr) {
            value = creator != nullptr ? creator() : new T();
            m_object->setProperty(propertyName, QVariant::fromValue(value));
        }
        return value;
    }

    SharedPointer<WidgetWrapperInjectedCommutatorData> InjectedCommutator(const char* propertyName, const std::function<void (QObject* w)>& handler = nullptr, qint32 delay = 1000) const
    {
        return Injected<WidgetWrapperInjectedCommutatorData>(propertyName, [&]() -> WidgetWrapperInjectedCommutatorData* {
            auto* result = new WidgetWrapperInjectedCommutatorData(delay);
            auto* widget = m_object;
            result->Commutator.Connect(CONNECTION_DEBUG_LOCATION, [handler, widget]{
                 handler(widget);
            });
            return result;
        });
    }

    template<class Property, typename ... Args>
    SharedPointer<Property> GetOrCreateProperty(const char* propName, const std::function<void (QObject*, const Property&)>& handler, Args... args) const
    {
        return Injected<Property>(propName, [&]() -> Property* {
            auto* property = new Property(args...);
            auto* widget = m_object;
            property->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget, handler, property]{ handler(widget, *property); });
            property->SetSetterHandler(ThreadHandlerMain);
            return property;
        });
    }

    template<class Widget, class Property, typename Signal, typename ... Args>
    SharedPointer<Property> GetOrCreateProperty(const char* propName, const std::function<void (Widget*, const Property&)>& setWidgetFromProperty,
                                                const std::function<void (Widget*, Property*)>& setPropertyFromWidget, Signal signal, const std::function<void (Widget*)>& initialization,
                                                Args... args) const
    {
        return Injected<Property>(propName, [&]() -> Property* {
            auto* property = new Property(args...);
            auto* widget = (Widget*)m_object;
            auto recursionGuard = ::make_shared<bool>(false);
            property->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget, setWidgetFromProperty, property, recursionGuard]{
                if(*recursionGuard) {
                    return;
                }
                guards::BooleanGuard guard(recursionGuard.get());
                setWidgetFromProperty(widget, *property);
            });
            widget->connect(widget, signal, [widget, property, recursionGuard, setPropertyFromWidget]{
                  if(*recursionGuard) {
                      return;
                  }
                  guards::BooleanGuard guard(recursionGuard.get());
                  setPropertyFromWidget(widget, property);
            });
            initialization(widget);
            property->SetSetterHandler(ThreadHandlerMain);
            return property;
        });
    }

protected:
    template<class T>
    const T& make(const std::function<void (const T&)>& handler) const
    {
        auto* tThis = reinterpret_cast<const T*>(this);
        handler(*tThis);
        return *tThis;
    }

protected:
    QObject* m_object;
};

class MainProgressBar;
class WidgetWrapper : public ObjectWrapper
{
    using Super = ObjectWrapper;
public:
    WidgetWrapper(QWidget* widget = nullptr);

    template<class T>
    T* InjectedWidget(const char* propertyName, const std::function<T* (QWidget* parent)>& creator = nullptr) const
    {
        auto* value = (T*)GetWidget()->property(propertyName).value<size_t>();
        if(value == nullptr) {
            value = creator != nullptr ? creator(GetWidget()) : new T(GetWidget());
            GetWidget()->setProperty(propertyName, (size_t)value);
        }
        return value;
    }

    const WidgetWrapper& Click();

    QByteArray StoreGeometry() const;
    bool RestoreGeometry(const QByteArray& geometry) const;

    void RegisterDialogView(const DescCustomDialogParams& params);

    template<typename ... Args>
    EventFilterObject* ConnectFocus(Args... other) const
    {
        auto* widget = GetWidget();
        return AddEventFilter([=](QObject*, QEvent* e) {
            switch(e->type()) {
            case QEvent::FocusIn:
                if(!WidgetWrapper(widget).WidgetEnablity()) {
                    return false;
                }
                adapters::Combine([](auto* w) {
                    WidgetWrapper(w).ApplyStyleProperty("a_focus", true);
                }, other...);
            break;
            case QEvent::FocusOut:
                if(!WidgetWrapper(widget).WidgetEnablity()) {
                    return false;
                }
                adapters::Combine([](auto* w) {
                    WidgetWrapper(w).ApplyStyleProperty("a_focus", false);
                }, other...);
            break;
            default: break;
            }
            return false;
        });
    }

    template<typename ... Args>
    void CreateFocusRule(Args... widgets)
    {
        auto* target = GetWidget();
        adapters::Combine([&](auto* w) {
            WidgetWrapper(w).ConnectFocus(target);
        }, widgets...);
    }

    DispatcherConnection ConnectEnablityFrom(const char* conInfo, QWidget* widget) const;
    DispatcherConnection ConnectVisibilityFrom(const char* conInfo, QWidget* widget) const;

    DispatcherConnection ConnectEnablityTo(const char* conInfo, QWidget* widget) const;
    DispatcherConnection ConnectVisibilityTo(const char* conInfo, QWidget* widget) const;

    static DispatcherConnections ConnectVisibilityToInt(const char* debugLocation, const LocalPropertyInt& mode, const QVector<QWidget*>& widgets);
    DispatcherConnections CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const;
    DispatcherConnections CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const;
    template<typename ... Dispatchers>
    DispatcherConnections CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        return createRule<WidgetWrapper>(debugLocation, QOverload<>::of(&WidgetWrapper::WidgetVisibility), handler, additionalWidgets, dispatchers...);
    }
    template<typename ... Dispatchers>
    DispatcherConnections CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        return createRule<WidgetWrapper>(debugLocation, &WidgetWrapper::WidgetEnablity, handler, additionalWidgets, dispatchers...);
    }

    void ActivateWindow(int mode, qint32 delay = 1000) const;
    void Highlight(qint32 unhightlightIn = 0) const;
    void Lowlight() const;
    void SetVisibleAnimated(bool visible, int duration = 1000, double opacity = 0.8) const;
    void ShowAnimated(int duration = 1000, double opacity = 0.8) const;
    void HideAnimated(int duration = 1000) const;

    class WidgetsLocationAttachment* LocateToParent(const DescWidgetsLocationAttachmentParams& params) const;
    WidgetsLocationAttachment* Location() const;

    void UpdateStyle(bool recursive = false) const;
    void ApplyStyleProperty(const char* propertyName, const QVariant& value, bool recursive = false) const;

    MainProgressBar* AddModalProgressBar(const Name& processId = Name()) const;
    MainProgressBar* AddModalProgressBar(const QSet<Name>& processIds) const;

    template<class T>
    T* AddModalProgressBar(const Name& processId) const
    {
        return AddModalProgressBar<T>(QSet<Name>{processId});
    }

    template<class T>
    T* AddModalProgressBar(const QSet<Name>& processIds) const
    {
        auto* progressBar = new T(processIds, GetWidget());
    #ifdef QT_DEBUG
        Q_ASSERT(!GetWidget()->property("a_progressBar").toBool());
        GetWidget()->setProperty("a_progressBar", true);
    #endif
        return progressBar;
    }

    const WidgetWrapper& AddToFocusManager(const QVector<QWidget*>& additionalWidgets) const;
    const WidgetWrapper& CreateCustomContextMenu(const std::function<void (QMenu*)>& creatorHandler, bool preventFromClosing = false) const;
    const WidgetWrapper& AddTestHandler(const FAction& testHandler) const;

    const WidgetWrapper& BlockWheel() const;
    const WidgetWrapper& FixUp() const;
    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetWrapper, QWidget)
    const WidgetWrapper& SetPalette(const QHash<qint32, LocalPropertyColor*>& palette) const;

    DispatcherConnectionsSafe& WidgetConnections() const;
    QVector<QWidget*>& WidgetTrueFocusWidgets() const;
    LocalPropertySequentialEnum<HighLightEnum>& WidgetHighlighted() const;
    LocalPropertyBool& WidgetVisibility() const;
    LocalPropertyBool& WidgetEnablity() const;
    LocalPropertyBool& WidgetCollapsing(Qt::Orientation orientation, qint32 initialWidth) const;
    TranslatedStringPtr WidgetToolTip() const;
    Dispatcher& OnClicked() const;
    const WidgetWrapper& SetOnClicked(const FAction& action) const;
    const WidgetWrapper& SetToolTip(const FTranslationHandler& toolTip) const;

    bool HasParent(const QWidget* parent) const;
    void ForeachParentWidget(const std::function<bool(const WidgetWrapper&)>& handler) const;
    void ForeachChildWidget(const std::function<void (const WidgetWrapper&)>& handler) const;

protected:
    template<typename T, typename FPropertyGetter, typename ... Dispatchers>
    DispatcherConnections createRule(const char* debugLocation, const FPropertyGetter& propertyGetter, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        DispatcherConnections result;
        auto& targetProperty = (Cast<T>().*propertyGetter)();
        result += targetProperty.ConnectFromDispatchers(debugLocation, [handler] { return handler(); }, dispatchers...);
        for(auto* widget : additionalWidgets) {
            result += (WidgetWrapper(widget).Cast<T>().*propertyGetter)().ConnectFrom(debugLocation, targetProperty);
        }
        return result;
    }

    template<typename T>
    const T& setChecked(bool checked) const { reinterpret_cast<const T*>(this)->WidgetChecked() = checked; return *(T*)this; }
    template<typename T, typename ... Dispatchers>
    const T& setText(const FTranslationHandler& handler, Dispatchers&... dispatchers) const
    {
        reinterpret_cast<const T*>(this)->WidgetText()->SetTranslationHandler(handler, dispatchers...);
        return *(T*)this;
    }
    template<typename T>
    const T& setOnClicked(const FAction& handler) const
    {
        auto connections = DispatcherConnectionsSafeCreate();
        reinterpret_cast<const T*>(this)->OnClicked().Connect(CONNECTION_DEBUG_LOCATION, [connections, handler]{ handler(); }).MakeSafe(*connections);
        return *(T*)this;
    }
    template<typename T>
    const T& setVisible(bool visible) const
    {
        WidgetVisibility() = visible;
        return *(T*)this;
    }
    template<typename T>
    const T& setEnabled(bool visible) const
    {
        WidgetEnablity() = visible;
        return *(T*)this;
    }
    template<typename T>
    const T& setHighlighted(bool visible) const
    {
        WidgetHighlighted() = visible;
        return *(T*)this;
    }
    template<typename T, typename ... Dispatchers>
    const T& setToolTip(const FTranslationHandler& handler, Dispatchers&... dispatchers) const
    {
        WidgetToolTip()->SetTranslationHandler(handler, dispatchers...);
        return *(T*)this;
    }
};

class WidgetScrollAreaWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetScrollAreaWrapper(class QScrollArea* button);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetScrollAreaWrapper, QScrollArea)

    WidgetScrollAreaWrapper& AddScrollByWheel(Qt::Orientation orientation);
};

class WidgetProgressBarWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetProgressBarWrapper(class QProgressBar* button);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetProgressBarWrapper, QProgressBar)

    WidgetProgressBarWrapper& SetId(const Name& processId);
};

class WidgetAbstractButtonWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetAbstractButtonWrapper(class QAbstractButton* button);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetAbstractButtonWrapper, QAbstractButton)
    DECLARE_WIDGET_WRAPPER_ADD_CHECKED(WidgetAbstractButtonWrapper)
    DECLARE_WIDGET_WRAPPER_ADD_CLICKED(WidgetAbstractButtonWrapper)
    DECLARE_WIDGET_WRAPPER_ADD_TEXT(WidgetAbstractButtonWrapper)

    const WidgetAbstractButtonWrapper& SetControl(ButtonRole i = ButtonRole::Icon, bool update = false) const;

    LocalPropertyBool& WidgetChecked() const;
    TranslatedStringPtr WidgetText() const;
    Dispatcher& OnClicked() const;
    const WidgetAbstractButtonWrapper& SetIcon(const Name& iconId) const;
};

using WidgetPushButtonWrapper = WidgetAbstractButtonWrapper;

class WidgetLineEditWrapper : public WidgetWrapper
{
public:
    WidgetLineEditWrapper(class QLineEdit* lineEdit);
    WidgetLineEditWrapper(class WidgetsLineEditLayout* lay);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetLineEditWrapper, QLineEdit)
    const WidgetLineEditWrapper& SetDynamicSizeAdjusting() const;

    const WidgetLineEditWrapper& AddCompleter(const QStringList& keys) const;
    const WidgetLineEditWrapper& AddPasswordButton() const;

    CommonDispatcher<const QString&>& OnEditFinished() const;
    LocalPropertyString& WidgetText() const;
    LocalPropertyBool& WidgetReadOnly() const;
private:
};

class WidgetTextEditWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetTextEditWrapper(class QTextEdit* lineEdit);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetTextEditWrapper, QTextEdit)

    const WidgetTextEditWrapper& AddCompleter(const QStringList& keys) const;

    QString Chopped(qint32 maxCount) const;
    LocalPropertyBool& WidgetReadOnly() const;

private:
};

class WidgetCheckBoxWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetCheckBoxWrapper(class QCheckBox* target);

    DECLARE_WIDGET_WRAPPER_ADD_CHECKED(WidgetCheckBoxWrapper)
    DECLARE_WIDGET_WRAPPER_ADD_TEXT(WidgetCheckBoxWrapper)

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetCheckBoxWrapper, QCheckBox)
    LocalPropertyBool& WidgetChecked() const;
    TranslatedStringPtr WidgetText() const;

    qint64 GetAssignedFlag() const;
    const WidgetCheckBoxWrapper& AssignFlag(qint64 flag) const;
};

class WidgetComboboxWrapper : public WidgetWrapper
{
public:
    WidgetComboboxWrapper(class QComboBox* combobox);
    WidgetComboboxWrapper(class WidgetsComboBoxLayout* combobox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetComboboxWrapper, QComboBox)
    template<class Enum, typename ... Dispatchers>
    const WidgetComboboxWrapper& SetEnum(const std::function<void (ModelsStandardListModelContainer&)>& handler = [](ModelsStandardListModelContainer&){}, Dispatchers&... dispatchers) const
    {
        auto* viewModel = ViewModelsStandardListModel::CreateEnumViewModel<Enum>(GetWidget(), handler, dispatchers...);
        GetWidget()->setModel(viewModel);
        return *this;
    }

    template<class Enum>
    const WidgetComboboxWrapper& SetCategorizedEnum(const ViewModelsCategoriesContainer<Enum>& categories) const
    {
        auto* viewModel = ViewModelsStandardListModel::CreateCategorizedEnumViewModel<Enum>(GetWidget(), categories);
        GetWidget()->setModel(viewModel);
        return *this;
    }

    template<typename T, class ... Args, typename = typename std::enable_if<std::is_enum<T>::value>::type>
    const WidgetComboboxWrapper& EnableStandardItems(const T& v, Args... args) const
    {
        QSet<qint32> result;
        adapters::Combine([&](auto type){
            result.insert((qint32)type);
        }, v, args...);
        return EnableStandardItems(result);
    }

    template<typename T, class ... Args, typename = typename std::enable_if<std::is_enum<T>::value>::type>
    const WidgetComboboxWrapper& DisableStandardItems(const T& v, Args... args) const
    {
        QSet<qint32> result;
        adapters::Combine([&](auto type){
            result.insert((qint32)type);
        }, v, args...);
        return DisableStandardItems(result);
    }

    const WidgetComboboxWrapper& EnableStandardItems(const QSet<qint32>& indices) const;
    const WidgetComboboxWrapper& DisableStandardItems(const QSet<qint32>& indices) const;
    const WidgetComboboxWrapper& DisconnectModel() const;
    const WidgetComboboxWrapper& AddViewModelEndEditHints(const std::function<void (QAbstractItemDelegate::EndEditHint)>& handler) const;
    class QCompleter* CreateCompleter(QAbstractItemModel* model, const std::function<void (const QModelIndex& index)>& onActivated, qint32 column = 0, QCompleter::ModelSorting sorting = QCompleter::CaseInsensitivelySortedModel) const;

    bool SetCurrentData(const QVariant& value, Qt::ItemDataRole role) const;

    CommonDispatcher<qint32>& OnActivated() const;

private:
    struct DisabledColumnComponentData& disabledColumnComponent() const;
};

class WidgetGroupboxWrapper : public WidgetWrapper
{
public:
    WidgetGroupboxWrapper(class QGroupBox* groupBox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetGroupboxWrapper, QGroupBox)
    LocalPropertyBool& WidgetChecked() const;
    const WidgetGroupboxWrapper& AddCollapsing() const;
    const WidgetGroupboxWrapper& AddCollapsingDispatcher(Dispatcher& updater, class QScrollArea* area = nullptr, qint32 delay = 1000) const;
};

class WidgetGroupboxLayoutWrapper : public WidgetWrapper
{
public:
    WidgetGroupboxLayoutWrapper(class WidgetsGroupBoxLayout* groupBox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetGroupboxLayoutWrapper, WidgetsGroupBoxLayout)
    LocalPropertyBool& WidgetChecked() const;
    const WidgetGroupboxLayoutWrapper& AddCollapsing() const;
    const WidgetGroupboxLayoutWrapper& AddCollapsingDispatcher(Dispatcher& updater, class QScrollArea* area = nullptr, qint32 delay = 1000) const;
};

class WidgetTabBarLayoutWrapper : public WidgetWrapper
{
public:
    WidgetTabBarLayoutWrapper(class WidgetsTabBarLayout* groupBox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetTabBarLayoutWrapper, WidgetsTabBarLayout)
    //LocalPropertyBool& WidgetChecked() const;
    const WidgetTabBarLayoutWrapper& AddCollapsing() const;
    const WidgetTabBarLayoutWrapper& AddCollapsingDispatcher(Dispatcher& updater, class QScrollArea* area = nullptr, qint32 delay = 1000) const;
};

class LocalPropertyDoubleDisplay : public LocalPropertyDouble
{
    using Super = LocalPropertyDouble;
public:
    LocalPropertyDoubleDisplay(double value = 0, double min = (std::numeric_limits<double>::lowest)(), double max = (std::numeric_limits<double>::max)())
        : Super(value, min, max)
        , Precision(2)
    {
        OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this]{
            DisplayValue.SetMinMax(*this, *this);
        });
    }

    LocalPropertyDoubleDisplay& operator-=(double value) { SetValue(Super::Native() - value); return *this; }
    LocalPropertyDoubleDisplay& operator+=(double value) { SetValue(Super::Native() + value); return *this; }
    LocalPropertyDoubleDisplay& operator=(double value) { SetValue(value); return *this; }

    LocalPropertyDouble DisplayValue;
    LocalPropertyInt Precision;
};

class WidgetLabelWrapper : public WidgetWrapper
{
public:
    WidgetLabelWrapper(class QLabel* label);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetLabelWrapper, QLabel)
    DECLARE_WIDGET_WRAPPER_ADD_TEXT(WidgetLabelWrapper)

    TranslatedStringPtr WidgetText() const;

    CommonDispatcher<const Name&>& OnLinkActivated() const;
};

class WidgetElidedLabelWrapper : public WidgetWrapper
{
public:
    WidgetElidedLabelWrapper(class ElidedLabel* label);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetElidedLabelWrapper, ElidedLabel)
    DECLARE_WIDGET_WRAPPER_ADD_TEXT(WidgetElidedLabelWrapper)

    TranslatedStringPtr WidgetText() const;
};

class WidgetTableViewWrapper : public WidgetWrapper
{
public:
    WidgetTableViewWrapper(class QTableView* tableView);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetTableViewWrapper, QTableView)
    void DebugJson() const;
    void DebugSelect() const;
    bool CopySelectedTableContentsToClipboard(bool includeHeaders = false) const;
    QList<int> SelectedRowsSorted() const;
    QList<int> SelectedColumnsSorted() const;
    QSet<int> SelectedRowsSet() const;
    QSet<int> SelectedColumnsSet() const;
    void SelectRowsAndScrollToFirst(const QSet<qint32>& rows) const;
    void SelectColumnsAndScrollToFirst(const QSet<qint32>& columns) const;
    WidgetsGlobalTableActionsScopeHandlersPtr SetDefaultActionHandlers(bool readOnly) const;
    WidgetsGlobalTableActionsScopeHandlersPtr CreateDefaultActionHandlers() const;
    WidgetsGlobalTableActionsScopeHandlersPtr GetActionHandlers() const;
    class QHeaderView* InitializeHorizontal(const DescTableViewParams& params = DescTableViewParams()) const;
    QHeaderView* InitializeVertical(const DescTableViewParams& params = DescTableViewParams()) const;
    class WidgetTableViewColumnsAttachment* AddColumnAttachment(const std::function<QWidget* (qint32 column)>& delegate) const;
    class WidgetsMatchingAttachment* CreateMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns) const;

    CommonDispatcher<qint32, qint32>& OnCurrentIndexChanged() const;
    const WidgetTableViewWrapper& SetOnCurrentIndexChanged(const std::function<void (qint32, qint32)>& handler) const;
};

class ActionWrapper : public ObjectWrapper
{
    using Super = ObjectWrapper;
public:
    ActionWrapper(QAction* action = nullptr);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS_BASE(ActionWrapper, QAction);
    const ActionWrapper& SetShortcut(const QKeySequence& keySequence) const;
    const ActionWrapper& SetText(const QString& text) const;
    const ActionWrapper& SetIcon(const Name& iconName) const;

    Dispatcher& OnClicked() const;

    QAction* GetAction() const { return GetWidget(); }

    LocalPropertyBool& WidgetVisibility() const;
    LocalPropertyBool& WidgetEnablity() const;
    TranslatedStringPtr WidgetText() const;
    LocalPropertyBool& WidgetChecked() const;
};

class WidgetSplitterWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetSplitterWrapper(class QSplitter* widget);

    const WidgetSplitterWrapper& SetWidgetSize(QWidget* widget, qint32 size) const;
    const WidgetSplitterWrapper& SetInner(bool repolish = false) const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetSplitterWrapper, QSplitter);
};

class WidgetDoubleSpinBoxWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetDoubleSpinBoxWrapper(class QDoubleSpinBox* widget);
    WidgetDoubleSpinBoxWrapper(class WidgetsDoubleSpinBoxLayout* widget);

    template<typename ... Dispatchers>
    DispatcherConnections CreateReadOnlyRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        return createRule<WidgetDoubleSpinBoxWrapper>(debugLocation, &WidgetDoubleSpinBoxWrapper::WidgetReadOnly, handler, additionalWidgets, dispatchers...);
    }

    LocalPropertyDouble& WidgetValue() const;
    LocalPropertyBool& WidgetReadOnly() const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetDoubleSpinBoxWrapper, QDoubleSpinBox);
};

class WidgetSpinBoxWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetSpinBoxWrapper(class QSpinBox* widget);
    WidgetSpinBoxWrapper(class WidgetsSpinBoxLayout* widget);

    LocalPropertyInt& WidgetValue() const;
    LocalPropertyBool& WidgetReadOnly() const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetSpinBoxWrapper, QSpinBox);
};

class WidgetColorDialogWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetColorDialogWrapper(class QColorDialog* dialog);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetColorDialogWrapper, QColorDialog)

    const WidgetColorDialogWrapper& SetDefaultLabels() const;
    const WidgetColorDialogWrapper& SetShowAlpha(bool show) const;
};

class WidgetDialogWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetDialogWrapper(const Name& id, const std::function<DescCustomDialogParams ()>& paramsCreator);

    template<class T>
    T* GetView() const { return GetWidget()->GetView<T>(); }
    qint32 Show(const DescShowDialogParams& params = DescShowDialogParams()) const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetDialogWrapper, WidgetsDialog)
};

class WidgetHeaderViewWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetHeaderViewWrapper(class QHeaderView* header);

    LocalPropertyBool& SectionVisibility(qint32 logicalIndex) const;
    const WidgetHeaderViewWrapper& MoveSection(qint32 logicalIndexFrom, qint32 logicalIndexTo) const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetHeaderViewWrapper, QHeaderView);
};

class MenuWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    MenuWrapper(QWidget* widget = nullptr, const WidgetsGlobalTableActionsScopeHandlersPtr& handlers = nullptr)
        : Super(widget)
        , m_globalActionsHandlers(handlers)
    {}

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(MenuWrapper, QMenu);
    const MenuWrapper& AddDebugActions() const;
    const MenuWrapper& AddGlobalAction(const QString& path) const;
    const MenuWrapper& AddGlobalTableAction(const Latin1Name& id) const;
    ActionWrapper AddAction(const QString& title, const std::function<void ()>& handle) const;
    ActionWrapper AddAction(const QString &title, const std::function<void (QAction*)> &handle) const;
    const MenuWrapper& SetIcon(const Name& iconName) const;

    LocalPropertyBool& WidgetVisibility() const;

    QMenu* GetMenu() const;

    template<class Property>
    ActionWrapper AddCheckboxAction(const QString& title, Property* value) const
    {
        return AddCheckboxAction(title, *value, [value](bool val){
            *value = val;
        });
    }
    template<class Property>
    ActionWrapper AddColorAction(const QString& title, Property* color) const
    {
        return AddColorAction(title, *color, [color](const QColor& val){
            *color = val;
        });
    }
    template<class Property>
    ActionWrapper AddDoubleAction(const QString& title, Property* value) const
    {
        return AddDoubleAction(title, value->GetMin(), value->GetMax(), *value, [value](const std::optional<double>& val){
            *value = val.value_or(0.0);
        });
    }
    template<class Property>
    ActionWrapper AddDoubleAction(const QString& title, LocalPropertyOptional<Property>* value) const
    {
        return AddDoubleAction(title, value->Value.GetMin(), value->Value.GetMax(), value->Native(), [value](const std::optional<double>& val){
            *value = val;
        });
    }

#ifdef UNITS_MODULE_LIB
    ActionWrapper AddMeasurementAction(const Measurement* measurement, const QString& title, LocalPropertyDouble* value) const;
#endif

    template<class Property>
    ActionWrapper AddIntAction(const QString& title, Property* value) const
    {
        return AddIntAction(title, *value, [value](qint32 val){
            *value = val;
        });
    }

    ActionWrapper AddCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler) const;
    ActionWrapper AddColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler) const;
    ActionWrapper AddDoubleAction(const QString& title, double min, double max, const std::optional<double>& value, const std::function<void (const std::optional<double>&)>& handler) const;
    ActionWrapper AddIntAction(const QString& title, qint32 value, const std::function<void (qint32 value)>& handler) const;
    ActionWrapper AddTableColumnsAction() const;
    ActionWrapper AddSeparator() const;
    class QMenu* AddPreventedFromClosingMenu(const QString& title) const;
    static QMenu* CreatePreventedFromClosingMenu(const QString& title, QWidget* parent = nullptr);
    QMenu* AddMenu(const QString& label) const;

    const MenuWrapper& SetOnContextMenu(const FAction& action) const;
    Dispatcher& OnContextMenu() const;

private:
    WidgetsGlobalTableActionsScopeHandlersPtr m_globalActionsHandlers;
};

class WidgetsObserver : public QObject
{
    WidgetsObserver();
public:
    static WidgetsObserver& GetInstance();

    void EnableAutoCollapsibleGroupboxes();

    CommonDispatcher<QWidget*> OnAdded;

private:
    bool eventFilter(QObject* watched, QEvent* e) override;
};

class ViewModelWrapper : public ObjectWrapper
{
    using Super = ObjectWrapper;
    Q_DECLARE_TR_FUNCTIONS(ViewModelWrapper);
public:
    using FIterationHandler = std::function<bool (const QModelIndex& index)>; // if returns true then interrupt iterating
    ViewModelWrapper(QAbstractItemModel* model);

    QAbstractItemModel* GetViewModel() const { return GetWidget(); }

    DECLARE_WIDGET_WRAPPER_FUNCTIONS_BASE(ViewModelWrapper, QAbstractItemModel);

    class ViewModelsDefaultFieldModel* CreateDefaultFieldModel(QObject* parent, const FTranslationHandler& field = nullptr) const;
    const ViewModelWrapper& ForeachModelIndex(const QModelIndex& parent, const FIterationHandler& function) const;
    const ViewModelWrapper& ForeachModelIndex(const FIterationHandler& function) const { return ForeachModelIndex(QModelIndex(), function); }
    qint32 IndexOf(const FIterationHandler& handler) const;
    QModelIndex Find(const FIterationHandler& handler) const;

    Dispatcher& OnReset() const;
};
#endif // WIDGETHELPERS_H
