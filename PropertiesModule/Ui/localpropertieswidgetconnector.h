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

protected:
    virtual void onClear() {}

private:
    StackPointers<LocalPropertiesWidgetConnectorBase> m_connectors;
};

template<class T, typename ... Args>
T* LocalPropertiesWidgetConnectorsContainer::AddConnector(Args... args)
{
    auto* connector = new T(args...);
    m_connectors.Append(connector);
    return connector;
}

class _Export LocalPropertiesWidgetConnectorBase : public QObject
{
    using Setter = std::function<void ()>;
public:
    LocalPropertiesWidgetConnectorBase(const Setter& widgetSetter, const Setter& propertySetter);

    void Update() { m_widgetSetter(); }

protected:
    friend class ChangeGuard;
    Setter m_widgetSetter;
    Setter m_propertySetter;
    QtLambdaConnections m_connections;
    DispatcherConnectionsSafe m_dispatcherConnections;
    bool m_ignorePropertyChange;
    bool m_ignoreWidgetChange;
};

class _Export LocalPropertiesPushButtonConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesPushButtonConnector(Dispatcher* dispatcher, class QPushButton* button);
    LocalPropertiesPushButtonConnector(LocalPropertyBool* checkedProperty, QPushButton* button);
    LocalPropertiesPushButtonConnector(LocalPropertyInt* property, const QVector<QPushButton*>& buttons);

private:
    qint32 m_currentIndex;
};

class _Export LocalPropertiesMenuLabelConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesMenuLabelConnector(LocalPropertyString* property, QMenu* menu);
};

class LocalPropertiesMenuActionsConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    template<class T>
    LocalPropertiesMenuActionsConnector(LocalPropertySequentialEnum<T>* property, QMenu* menu, const std::function<void (QAction* action, qint32 index)>& actionsRules = [](QAction*,qint32){})
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

protected:
};

class _Export LocalPropertiesComboBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    template<class Enum>
    LocalPropertiesComboBoxConnector(LocalPropertySequentialEnum<Enum>* property, class QComboBox* comboBox)
        : LocalPropertiesComboBoxConnector(property, comboBox, (qint32)Enum::First)
    {}
    LocalPropertiesComboBoxConnector(LocalPropertyInt* property, QComboBox* comboBox, qint32 offset = 0);
    LocalPropertiesComboBoxConnector(LocalProperty<Name>* property, QComboBox* comboBox);
    template<class T>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, QComboBox* comboBox)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox)
    {}
#ifdef WIDGETS_MODULE_LIB
    template<class T>
    LocalPropertiesComboBoxConnector(StateParameterProperty<T>* property, QComboBox* comboBox, const SharedPointer<class ModelsStandardListModel>& model)
        : LocalPropertiesComboBoxConnector(&property->InputValue, comboBox, model)
    {}
    LocalPropertiesComboBoxConnector(LocalPropertyInt* property, QComboBox* comboBox, const SharedPointer<class ModelsStandardListModel>& model);
    LocalPropertiesComboBoxConnector(LocalProperty<Name>* property, QComboBox* combobox, const SharedPointer<ModelsStandardListModel>& model);
#endif

private:
    void connectComboBox(QComboBox* combobox);
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

#ifdef WIDGETS_MODULE_LIB
    template<class T>
    LocalPropertiesSpinBoxConnector(StateParameterProperty<T>* property, WidgetsDoubleSpinBoxWithCustomDisplay* spin)
        : LocalPropertiesSpinBoxConnector(&property->InputValue, spin)
    {}
    LocalPropertiesSpinBoxConnector(LocalPropertyIntOptional* property, class WidgetsSpinBoxWithCustomDisplay* spinBox);
#endif
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

#ifdef WIDGETS_MODULE_LIB
    template<class T>
    LocalPropertiesDoubleSpinBoxConnector(StateParameterProperty<T>* property, WidgetsDoubleSpinBoxWithCustomDisplay* spin)
        : LocalPropertiesDoubleSpinBoxConnector(&property->InputValue, spin)
    {}
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox);
    LocalPropertiesDoubleSpinBoxConnector(class LocalPropertyDoubleDisplay* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox);
#endif
};

class _Export LocalPropertiesRadioButtonsConnector : public LocalPropertiesWidgetConnectorBase
{
    Q_OBJECT
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesRadioButtonsConnector(LocalPropertyInt* property, const Stack<class QRadioButton*>& buttons);

private:
    qint32 m_currentIndex;
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

#ifdef WIDGETS_MODULE_LIB

class _Export LocalPropertiesDateConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateConnector(LocalPropertyDate* property, class WidgetsDateEdit* dateTime);
    LocalPropertiesDateConnector(LocalPropertyDateTime* property, WidgetsDateEdit* dateTime);
};

class _Export LocalPropertiesDateTimeConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, class WidgetsDateTimeEdit* dateTime);
};
#endif

#endif // LOCALPROPERTIESWIDGETCONNECTOR_H
