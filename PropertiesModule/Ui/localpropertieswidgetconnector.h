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
    m_connectors.Append(connector);
    return connector;
}

class _Export LocalPropertiesWidgetConnectorBase : public QObject
{
public:
    LocalPropertiesWidgetConnectorBase(const FAction& widgetSetter, const FAction& propertySetter, QWidget* w = nullptr);

    LocalPropertyBool ForceDisabled;

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
    LocalPropertiesPushButtonConnector(Dispatcher* dispatcher, class QAbstractButton* button);
    LocalPropertiesPushButtonConnector(LocalPropertyBool* checkedProperty, QAbstractButton* button);
    LocalPropertiesPushButtonConnector(LocalPropertyInt* property, const QVector<QAbstractButton*>& buttons);

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
    template<class T>
    LocalPropertiesLabelConnector(StateParameterProperty<T>* property, class QLabel* label)
        : LocalPropertiesLabelConnector(&property->InputValue, label)
    {}
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

class _Export LocalPropertiesComboBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesComboBoxConnector(LocalPropertyName* property, QComboBox* combo);
    template<class Enum>
    LocalPropertiesComboBoxConnector(LocalPropertySequentialEnum<Enum>* property, QComboBox* combo)
        : LocalPropertiesComboBoxConnector(property, combo, IdRole)
    {}
    LocalPropertiesComboBoxConnector(LocalPropertyInt* property, QComboBox* combo);
    template<class T, typename value_type = typename T::value_type>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, QComboBox* comboBox)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox)
    {}
    template<class T, typename value_type = typename T::value_type>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, QComboBox* comboBox, qint32 role)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox, role)
    {}
    template<class T, typename value_type = typename T::value_type>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, QComboBox* comboBox, const std::function<value_type (const QModelIndex&)>& getter)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox, getter)
    {}
    template<class T>
    LocalPropertiesComboBoxConnector(T* property, QComboBox* comboBox, qint32 role)
        : LocalPropertiesComboBoxConnector(property, comboBox, [role](const QModelIndex& index) -> typename T::value_type { return index.data(role).value<typename T::value_type>(); })
    {}
    template<class T>
    LocalPropertiesComboBoxConnector(T* property, QComboBox* comboBox, const std::function<typename T::value_type (const QModelIndex&)>& getter)
        : Super([property, comboBox, getter]{
                    setCurrentIndex(comboBox, [property, getter](const QModelIndex& index) -> bool { return property->operator==(getter(index)); });
                },
                [property, comboBox, getter]{
                    *property = getter(currentIndex(comboBox));
                }, comboBox
        )
    {
        property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
            m_widgetSetter();
        }).MakeSafe(m_dispatcherConnections);

        connectComboBox(comboBox);
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

     template<class T>
     LocalPropertiesLineEditConnector(StateParameterProperty<T>* property, class QLineEdit* lineEdit, bool reactive = true)
         : LocalPropertiesLineEditConnector(&property->InputValue, lineEdit, reactive)
     {}

 private:
     DelayedCallObject m_textChanged;
};

class _Export LocalPropertiesSpinBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesSpinBoxConnector(LocalPropertyInt* property, class QSpinBox* spinBox);
    template<class T>
    LocalPropertiesSpinBoxConnector(StateParameterProperty<T>* property, QSpinBox* spin)
        : LocalPropertiesSpinBoxConnector(&property->InputValue, spin)
    {}

    template<class T>
    LocalPropertiesSpinBoxConnector(StateParameterProperty<T>* property, WidgetsDoubleSpinBoxWithCustomDisplay* spin)
        : LocalPropertiesSpinBoxConnector(&property->InputValue, spin)
    {}
    LocalPropertiesSpinBoxConnector(LocalPropertyIntOptional* property, class WidgetsSpinBoxWithCustomDisplay* spinBox);

};

class _Export LocalPropertiesDoubleSpinBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, class QDoubleSpinBox* spinBox, const std::function<void (double)>& propertySetter = nullptr);
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, QDoubleSpinBox* spinBox);
    template<class T>
    LocalPropertiesDoubleSpinBoxConnector(StateParameterProperty<T>* property, QDoubleSpinBox* spin)
        : LocalPropertiesDoubleSpinBoxConnector(&property->InputValue, spin)
    {}

    template<class T>
    LocalPropertiesDoubleSpinBoxConnector(StateParameterProperty<T>* property, WidgetsDoubleSpinBoxWithCustomDisplay* spin)
        : LocalPropertiesDoubleSpinBoxConnector(&property->InputValue, spin)
    {}
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox);
    LocalPropertiesDoubleSpinBoxConnector(class LocalPropertyDoubleDisplay* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox);
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
    
private:
    DelayedCallObject m_textChanged;
};

class _Export LocalPropertiesDateConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateConnector(LocalPropertyDate* property, class WidgetsDateEdit* dateTime);
    LocalPropertiesDateConnector(LocalPropertyDateTime* property, WidgetsDateEdit* dateTime, LocalPropertyDoubleOptional* timeShift = nullptr);
};

class _Export LocalPropertiesDateTimeConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, class WidgetsDateTimeEdit* dateTime, LocalPropertyDoubleOptional* timeShift = nullptr);
};

#endif // LOCALPROPERTIESWIDGETCONNECTOR_H
