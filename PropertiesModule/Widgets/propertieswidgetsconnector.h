#ifndef PROPERTIESWIDGETSCONNECTOR_H
#define PROPERTIESWIDGETSCONNECTOR_H

/*
 *
 * Note: if you delete Connector then property should be deleted also
 * Note: Usually there is no necessity to delete Connector, expecialy if it reffered to Global context index scope
*/

#ifdef QT_GUI_LIB

#include "PropertiesModule/propertypromise.h"

class PropertiesConnectorBase;
class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;
class QRadioButton;

class _Export PropertiesConnectorsContainer
{
public:
    PropertiesConnectorsContainer()
    {}

    void AddConnector(PropertiesConnectorBase* connector);
    void Update();
    void Clear();
    bool IsEmpty() const { return m_connectors.IsEmpty(); }

private:
    StackPointers<PropertiesConnectorBase> m_connectors;
};

class _Export PropertiesConnectorContextIndexGuard
{
    properties_context_index_t m_prevContextIndex;
public:
    explicit PropertiesConnectorContextIndexGuard(properties_context_index_t contextIndex);
    explicit PropertiesConnectorContextIndexGuard();
    ~PropertiesConnectorContextIndexGuard();

    template<class T> static PropertyPromise<T> GetProperty(const Name& name)
    {
        return PropertiesSystem::GetProperty<T>(name, currentContextIndex());
    }

private:
    friend class PropertiesConnectorBase;
    static properties_context_index_t& currentContextIndex();
};

class _Export PropertiesConnectorBase : public QObject
{
    typedef std::function<void (const QVariant& )> Setter;   
public:
    PropertiesConnectorBase(const Name& name, const Setter& setter, QWidget* target);
    virtual ~PropertiesConnectorBase();

    void Update();

protected:
    friend class PropertiesConnectorsContainer;
    Setter m_setter;
    PropertyPtr m_propertyPtr;
    QMetaObject::Connection m_connection;
    bool m_ignorePropertyChange;

protected:
    class PropertyChangeGuard
    {
        bool& m_ignorePropertyChange;
    public:
        PropertyChangeGuard(PropertiesConnectorBase* connector);
        ~PropertyChangeGuard();
    };
};

class _Export PropertiesCheckBoxConnector : public PropertiesConnectorBase
{
public:
    PropertiesCheckBoxConnector(const Name& propertyName, QCheckBox* checkBox);
};

class _Export PropertiesLineEditConnector : public PropertiesConnectorBase
{
public:
    PropertiesLineEditConnector(const Name& propertyName, QLineEdit* lineEdit);
};

class _Export PropertiesSpinBoxConnector : public PropertiesConnectorBase
{
public:
    PropertiesSpinBoxConnector(const Name& propertyName, QSpinBox* spinBox);
};

class _Export PropertiesDoubleSpinBoxConnector : public PropertiesConnectorBase
{
public:
    PropertiesDoubleSpinBoxConnector(const Name& propertyName, QDoubleSpinBox* spinBox);
};

class _Export PropertiesTextEditConnector : public PropertiesConnectorBase
{
public:
    PropertiesTextEditConnector(const Name& propertyName, class QTextEdit* textEdit);
};

// Bool property
class _Export PropertiesGroupBoxConnector : public PropertiesConnectorBase
{
public:
    PropertiesGroupBoxConnector(const Name& propertyName, class QGroupBox* groupBox);
};

class _Export PropertiesRadioButtonsGroupBoxConnector : public PropertiesConnectorBase
{
    Q_OBJECT
public:
    PropertiesRadioButtonsGroupBoxConnector(const Name& propertyName, QGroupBox* groupBox, const Stack<QRadioButton*>& buttons);

    static Stack<QRadioButton*> ButtonsFromGroup(QGroupBox* groupBox);

signals:
    void valueChanged(qint32 newValue);
};

#endif // QT_GUI_LIB

#endif // PROPERTIESWIDGETSCONNECTOR_H
