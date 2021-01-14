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

class _Export LocalPropertyLabelConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertyLabelConnector(LocalPropertyString* property, class QLabel* label);
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
     LocalPropertiesLineEditConnector(LocalProperty<QString>* property, class QLineEdit* lineEdit);
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
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, class QDoubleSpinBox* spinBox);
    LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, QDoubleSpinBox* spinBox);
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
