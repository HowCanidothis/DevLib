#ifndef LOCALPROPERTIESWIDGETCONNECTOR_H
#define LOCALPROPERTIESWIDGETCONNECTOR_H

#include <SharedModule/internal.hpp>

#include "PropertiesModule/localproperty.h"

class _Export PropertiesWidgetConnectorBaseEx : public QObject
{
    using Setter = std::function<void ()>;
public:
    PropertiesWidgetConnectorBaseEx(const Setter& widgetSetter, const Setter& propertySetter);
    ~PropertiesWidgetConnectorBaseEx();

protected:
    friend class ChangeGuard;
    Setter m_widgetSetter;
    Setter m_propertySetter;
    QMetaObject::Connection m_connection;
    DispatchersConnections m_dispatcherConnections;
    bool m_ignorePropertyChange;

    class ChangeGuard : public guards::LambdaGuard
    {
        using Super = guards::LambdaGuard;
    public:
        ChangeGuard(PropertiesWidgetConnectorBaseEx* connector)
            : Super([connector]{ connector->m_ignorePropertyChange = false; })
        {
            connector->m_ignorePropertyChange = true;
        }
    };
};

class _Export LocalPropertiesCheckBoxConnector : public PropertiesWidgetConnectorBaseEx
{
    using Super = PropertiesWidgetConnectorBaseEx;
public:
    LocalPropertiesCheckBoxConnector(LocalProperty<bool>& property, class QCheckBox* checkBox);

protected:
};

class _Export LocalPropertiesLineEditConnector : public PropertiesWidgetConnectorBaseEx
{
    using Super = PropertiesWidgetConnectorBaseEx;
public:
     LocalPropertiesLineEditConnector(LocalProperty<QString>& property, class QLineEdit* lineEdit);
};

class _Export LocalPropertiesSpinBoxConnector : public PropertiesWidgetConnectorBaseEx
{
    using Super = PropertiesWidgetConnectorBaseEx;
public:
    LocalPropertiesSpinBoxConnector(LocalPropertyInt& property, class QSpinBox* spinBox);
};

class _Export LocalPropertiesDoubleSpinBoxConnector : public PropertiesWidgetConnectorBaseEx
{
    using Super = PropertiesWidgetConnectorBaseEx;
public:
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble& property, class QDoubleSpinBox* spinBox);
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat& property, QDoubleSpinBox* spinBox);
};

class _Export LocalPropertiesTextEditConnector : public PropertiesWidgetConnectorBaseEx
{
    using Super = PropertiesWidgetConnectorBaseEx;
public:
    enum SubmitType {
        SubmitType_None,
        SubmitType_OnEveryChange,
    };
    LocalPropertiesTextEditConnector(LocalProperty<QString>& property, class QTextEdit* textEdit, SubmitType submitType = SubmitType_OnEveryChange);
};

#endif // LOCALPROPERTIESWIDGETCONNECTOR_H
