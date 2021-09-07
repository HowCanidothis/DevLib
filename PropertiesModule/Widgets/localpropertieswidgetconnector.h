#ifndef LOCALPROPERTIESWIDGETCONNECTOR_H
#define LOCALPROPERTIESWIDGETCONNECTOR_H

#ifdef QT_GUI_LIB

#include <SharedModule/internal.hpp>
#include <SharedModule/External/external.hpp>

#include "PropertiesModule/localproperty.h"

class LocalPropertiesWidgetConnectorBase;

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
        for(const auto& name : property->GetNames()) {
            auto* action = createAction(name, [property, value]{
                *property = (T)value;
            }, menu);
            actionsRules(action, value);
            value++;
        }
    }
};

class _Export LocalPropertiesLabelConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesLabelConnector(LocalPropertyString* property, class QLabel* label);
    template<class T>
    LocalPropertiesLabelConnector(LocalPropertySequentialEnum<T>* property, QLabel* label)
        : Super([label, property]{
            label->setText(EnumHelper<T>::GetNames()[(qint32)property->Native()]);
        }, []{})
    {
        property->GetDispatcher().Connect(this, [this]{
            m_widgetSetter();
        }).MakeSafe(m_dispatcherConnections);
    }
};

class _Export LocalPropertiesCheckBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesCheckBoxConnector(LocalProperty<bool>* property, class QCheckBox* checkBox);

protected:
};

class _Export LocalPropertiesComboBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesComboBoxConnector(LocalPropertyInt* property, class QComboBox* comboBox);
};

class _Export LocalPropertiesLineEditConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
     LocalPropertiesLineEditConnector(LocalProperty<QString>* property, class QLineEdit* lineEdit, bool reactive = true);

 private:
     DelayedCallObject m_textChanged;
};

class _Export LocalPropertiesSpinBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesSpinBoxConnector(LocalPropertyInt* property, class QSpinBox* spinBox);
};

class _Export LocalPropertiesDoubleSpinBoxConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, class QDoubleSpinBox* spinBox, double presicion = 0.009);
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, QDoubleSpinBox* spinBox, float presicion = 0.009f);

#ifdef WIDGETS_MODULE_LIB
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleOptional* property, class WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double presicion = 0.009);
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

class _Export LocalPropertiesDateConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateConnector(LocalProperty<QDate>* property, class QDateEdit* dateTime);
};

class _Export LocalPropertiesDateTimeConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesDateTimeConnector(LocalProperty<QDateTime>* property, class QDateTimeEdit* dateTime);
};
#endif

#endif // LOCALPROPERTIESWIDGETCONNECTOR_H
