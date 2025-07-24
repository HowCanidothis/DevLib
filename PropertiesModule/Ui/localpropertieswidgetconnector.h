#ifndef LOCALPROPERTIESWIDGETCONNECTOR_H
#define LOCALPROPERTIESWIDGETCONNECTOR_H

#include <SharedModule/internal.hpp>
#include <SharedModule/External/external.hpp>

#include "PropertiesModule/localproperty.h"

class LocalPropertiesWidgetConnectorBase;
class WidgetsDoubleSpinBoxWithCustomDisplay;

class _Export LocalPropertiesWidgetConnectorsContainer
{
public:
    LocalPropertiesWidgetConnectorsContainer()
    {}

    template<class T, typename ... Args>
    T* AddConnector(Args... args);
    //void Update();
    void Clear();
    bool IsEmpty() const { return m_connectors.IsEmpty(); }

    LocalPropertyBool ForceDisabled;
    Dispatcher OnAboutToBeChanged;

protected:
    virtual void onClear() {}

private:
    StackPointers<LocalPropertiesWidgetConnectorBase> m_connectors;
};

template<class T, typename ... Args>
T* LocalPropertiesWidgetConnectorsContainer::AddConnector(Args... args)
{
    auto* connector = new T(args...);
    connector->ForceDisabled.ConnectFrom(CDL, ForceDisabled).MakeSafe(connector->m_dispatcherConnections);
    OnAboutToBeChanged.ConnectFrom(CDL, connector->OnAboutToBeChanged);
    m_connectors.Append(connector);
    return connector;
}

class _Export LocalPropertiesWidgetConnectorBase : public QObject
{
public:
    LocalPropertiesWidgetConnectorBase(const FAction& widgetSetter, const FAction& propertySetter, QWidget* w = nullptr);

    void Update();

    LocalPropertyBool ForceDisabled;
    Dispatcher OnAboutToBeChanged;

protected:
    friend class LocalPropertiesWidgetConnectorsContainer;
    friend class ChangeGuard;
    FAction m_widgetSetter;
    FAction m_propertySetter;
    QtLambdaConnections m_connections;
    DispatcherConnectionsSafe m_dispatcherConnections;
    bool m_ignorePropertyChange;
    bool m_ignoreWidgetChange;
};

class _Export LocalPropertiesPushButtonConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesPushButtonConnector(class QAbstractButton* button, const FAction& onClicked);
    LocalPropertiesPushButtonConnector(Dispatcher* dispatcher, QAbstractButton* button);
    LocalPropertiesPushButtonConnector(LocalPropertyBool* checkedProperty, QAbstractButton* button);
    LocalPropertiesPushButtonConnector(LocalPropertyInt* property, const QVector<class QPushButton*>& buttons);
    LocalPropertiesPushButtonConnector(LocalPropertyInt* property, const QVector<QAbstractButton*>& buttons);
    LocalPropertiesPushButtonConnector(LocalPropertyColor* property, class WidgetsColorPicker* button);

private:
    qint32 m_currentIndex;
};

class _Export LocalPropertiesMenuLabelConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesMenuLabelConnector(LocalPropertyString* property, class QMenu* menu);
};

class LocalPropertiesMenuActionsConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    template<class T>
    LocalPropertiesMenuActionsConnector(LocalPropertySequentialEnum<T>* property, QMenu* menu, const std::function<void (class QAction* action, qint32 index)>& actionsRules = [](QAction*,qint32){})
        : Super([]{
        }, []{})
    {
        qint32 value = (qint32)T::First;
        MenuWrapper MenuWrapper(menu);
        for(const auto& name : property->GetNames()) {
            auto action = MenuWrapper.AddAction(name, [property, value]{
                *property = (T)value;
            });
            actionsRules(action, value);
            value++;
        }
    }
};

class _Export LocalPropertiesLabelConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    struct ImageConnectorParams
    {
        QAbstractButton* BrowseButton = nullptr;
        QAbstractButton* ClearButton = nullptr;
        QString ForceDefaultDir;

        ImageConnectorParams& SetBrowseButton(QAbstractButton* browse)
        {
            BrowseButton = browse;
            return *this;
        }

        ImageConnectorParams& SetClearButton(QAbstractButton* clear)
        {
            ClearButton = clear;
            return *this;
        }

        ImageConnectorParams& SetForceDefaultBrowseDir(const QString& dir)
        {
            ForceDefaultDir = dir;
            return *this;
        }

    };

    template<class T>
    LocalPropertiesLabelConnector(StateParameterProperty<T>* property, class QLabel* label)
        : LocalPropertiesLabelConnector(&property->InputValue, label)
    {}

    LocalPropertiesLabelConnector(LocalPropertyString* fileName, LocalProperty<QByteArray>* imageSource, QLabel* label, const ImageConnectorParams& params); // Image
    LocalPropertiesLabelConnector(LocalPropertyString* property, class ElidedLabel* label);
    LocalPropertiesLabelConnector(LocalPropertyString* property, QLabel* label);
    template<class T>
    LocalPropertiesLabelConnector(LocalPropertySequentialEnum<T>* property, QLabel* label)
        : Super([label, property]{
            label->setText(property->GetName());
        }, []{})
    {
        property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
            m_widgetSetter();
        }).MakeSafe(m_dispatcherConnections);
    }
};

class _Export LocalPropertiesCheckBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesCheckBoxConnector(LocalPropertyBool* property, class WidgetsSwitchLayout* l);
    LocalPropertiesCheckBoxConnector(LocalPropertyBool* property, class QCheckBox* checkBox);
    LocalPropertiesCheckBoxConnector(LocalPropertyString* property, QCheckBox* checkBox);
    template<class T, typename CheckBoxWrapper, typename value_type = T::value_type>
    LocalPropertiesCheckBoxConnector(T* property, const QVector<CheckBoxWrapper>& checkBoxes)
        : Super([checkBoxes, property]{
                    value_type value = property->Native();
                    for(const auto& checkbox : checkBoxes) {
                        Q_ASSERT(checkbox.GetAssignedFlag() != 0);
                        checkbox->setChecked((value & checkbox.GetAssignedFlag()) == checkbox.GetAssignedFlag());
                    }
                },
                [property, checkBoxes]{
                    value_type value = 0;
                    for(const auto& checkbox : checkBoxes) {
                        if(checkbox->isChecked()) {
                            value |= checkbox.GetAssignedFlag();
                        }
                    }
                    *property = value;
                })
    {
        property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
            m_widgetSetter();
        }).MakeSafe(m_dispatcherConnections);

        for(const auto& checkbox : checkBoxes) {
            checkbox.WidgetChecked().OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
                ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[this]{
                    m_propertySetter();
                    m_widgetSetter();
                });
            }).MakeSafe(m_dispatcherConnections);
            checkbox.WidgetChecked() = false;
        }
    }

protected:
};

struct LocalPropertiesComboBoxConnectorExtractor
{
    LocalPropertiesComboBoxConnectorExtractor(class WidgetsComboBoxLayout* l);
    LocalPropertiesComboBoxConnectorExtractor(QComboBox* c);

    QComboBox* ComboBox;
};

class _Export LocalPropertiesComboBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesComboBoxConnector(LocalPropertyName* property, const LocalPropertiesComboBoxConnectorExtractor& combo);
    template<class Enum>
    LocalPropertiesComboBoxConnector(LocalPropertySequentialEnum<Enum>* property, const LocalPropertiesComboBoxConnectorExtractor& combo)
        : LocalPropertiesComboBoxConnector(property, combo, IdRole)
    {}
    LocalPropertiesComboBoxConnector(LocalPropertyInt* property, const LocalPropertiesComboBoxConnectorExtractor& combo);
    template<class T, typename value_type = typename T::value_type>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, const LocalPropertiesComboBoxConnectorExtractor& comboBox)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox)
    {}
    template<class T, typename value_type = typename T::value_type>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, const LocalPropertiesComboBoxConnectorExtractor& comboBox, qint32 role)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox, role)
    {}
    template<class T, typename value_type = typename T::value_type>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, const LocalPropertiesComboBoxConnectorExtractor& comboBox, const std::function<value_type (const QModelIndex&)>& getter)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox, getter)
    {}
    template<class T>
    LocalPropertiesComboBoxConnector(T* property, const LocalPropertiesComboBoxConnectorExtractor& comboBox, qint32 role)
        : LocalPropertiesComboBoxConnector(property, comboBox, [role](const QModelIndex& index) -> typename T::value_type { return index.data(role).value<typename T::value_type>(); })
    {}
    template<class T>
    LocalPropertiesComboBoxConnector(T* property, const LocalPropertiesComboBoxConnectorExtractor& comboBox, const std::function<typename T::value_type (const QModelIndex&)>& getter)
        : Super([property, comboBox, getter]{
                   setCurrentIndex(comboBox.ComboBox, [property, getter](const QModelIndex& index) -> bool {
                        return property->operator==(getter(index));
                   });
                },
                [property, comboBox, getter]{
                    *property = getter(currentIndex(comboBox.ComboBox));
                }, comboBox.ComboBox
        )
    {
        property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
            m_widgetSetter();
        }).MakeSafe(m_dispatcherConnections);

        connectComboBox(comboBox.ComboBox);
    }

private:
    void connectComboBox(QComboBox* combobox);
    static void setCurrentIndex(QComboBox* combobox, const std::function<bool (const QModelIndex&)>& handler);
    static QModelIndex currentIndex(QComboBox* combobox);
};

class _Export LocalPropertiesLineEditConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
     LocalPropertiesLineEditConnector(LocalProperty<QString>* property, class QLineEdit* lineEdit, bool reactive = true);
     LocalPropertiesLineEditConnector(LocalProperty<QString>* property, class WidgetsLineEditLayout* lineEdit, bool reactive = true);

     template<class T>
     LocalPropertiesLineEditConnector(StateParameterProperty<T>* property, class QLineEdit* lineEdit, bool reactive = true)
         : LocalPropertiesLineEditConnector(&property->InputValue, lineEdit, reactive)
     {}

 private:
     ScopedPointer<DelayedCallObject> m_textChanged;
};

struct LocalPropertiesSpinBoxConnectorExtractor
{
    LocalPropertiesSpinBoxConnectorExtractor(class WidgetsSpinBoxLayout* l);
    LocalPropertiesSpinBoxConnectorExtractor(class WidgetsSpinBoxWithCustomDisplay* s);

    WidgetsSpinBoxWithCustomDisplay* SpinBox;
    WidgetsSpinBoxWithCustomDisplay* operator->() const { return SpinBox; }
};

class _Export LocalPropertiesSpinBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesSpinBoxConnector(LocalPropertyInt* property, const LocalPropertiesSpinBoxConnectorExtractor& spinBox);
    template<class T>
    LocalPropertiesSpinBoxConnector(StateParameterProperty<T>* property, const LocalPropertiesSpinBoxConnectorExtractor& spin)
        : LocalPropertiesSpinBoxConnector(&property->InputValue, spin)
    {}
    LocalPropertiesSpinBoxConnector(LocalPropertyIntOptional* property, const LocalPropertiesSpinBoxConnectorExtractor& spinBox);

};

struct LocalPropertiesDoubleSpinBoxConnectorExtractor
{
    LocalPropertiesDoubleSpinBoxConnectorExtractor(class WidgetsDoubleSpinBoxLayout* l);
    LocalPropertiesDoubleSpinBoxConnectorExtractor(class WidgetsDoubleSpinBoxWithCustomDisplay* s);

    WidgetsDoubleSpinBoxWithCustomDisplay* SpinBox;
    WidgetsDoubleSpinBoxWithCustomDisplay* operator->() const { return SpinBox; }
};

class _Export LocalPropertiesDoubleSpinBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, const std::function<void (double)>& propertySetter = nullptr);
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox);

    template<class T>
    LocalPropertiesDoubleSpinBoxConnector(StateParameterProperty<T>* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox)
        : LocalPropertiesDoubleSpinBoxConnector(&property->InputValue, spin)
    {}
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox);
    LocalPropertiesDoubleSpinBoxConnector(class LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox);
};

class _Export LocalPropertiesTextEditConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    enum SubmitType {
        SubmitType_None,
        SubmitType_OnEveryChange,
    };
    LocalPropertiesTextEditConnector(LocalProperty<QString>* property, class QTextEdit* textEdit, SubmitType submitType = SubmitType_OnEveryChange);
    LocalPropertiesTextEditConnector(LocalProperty<QString>* property, class WidgetsTextEditLayout* textEdit, SubmitType submitType = SubmitType_OnEveryChange);
    
private:
    DelayedCallObject m_textChanged;
};

struct LocalPropertiesDateTimeConnectorExtractor
{
    LocalPropertiesDateTimeConnectorExtractor(class WidgetsDateTimeEdit* l);
    LocalPropertiesDateTimeConnectorExtractor(class WidgetsDateTimeLayout* s);

    WidgetsDateTimeEdit* DateTime;
    WidgetsDateTimeEdit* operator->() const { return DateTime; }
};

class _Export LocalPropertiesDateTimeConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateTimeConnector(LocalPropertyDate* property, const LocalPropertiesDateTimeConnectorExtractor& dateTime);
    LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, const LocalPropertiesDateTimeConnectorExtractor& dateTime, LocalPropertyDoubleOptional* timeShift = nullptr);
};

#endif // LOCALPROPERTIESWIDGETCONNECTOR_H
