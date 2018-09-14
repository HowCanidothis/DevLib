#ifndef PROPERTIESWIDGETSCONNECTOR_H
#define PROPERTIESWIDGETSCONNECTOR_H

/*
 *
 * Note: if you delete Connector then property should be deleted also
 * Note: Usually there is no necessity to delete Connector, expecialy if it reffered to Global context index scope
*/

#include "PropertiesModule/propertypromise.h"

class PropertiesConnectorBase;
class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;

class _Export PropertiesConnectorsContainer
{
public:
    PropertiesConnectorsContainer()
    {}

    void AddConnector(PropertiesConnectorBase* connector);
    void Update();
    void Clear();

    void Save();
    void Restore();

private:
    StackPointers<PropertiesConnectorBase> _connectors;
};

class _Export PropertiesConnectorContextIndexGuard
{
    properties_context_index_t _before;
public:
    explicit PropertiesConnectorContextIndexGuard(properties_context_index_t contextIndex);
    explicit PropertiesConnectorContextIndexGuard();
    ~PropertiesConnectorContextIndexGuard();

private:
    friend class PropertiesConnectorBase;
    static properties_context_index_t& currentContextIndex();
};

class _Export PropertiesConnectorBase : public QObject
{
    typedef std::function<void (const QVariant& )> Setter;
public:
    PropertiesConnectorBase(const Name& name, const Setter& setter, QObject* target);
    virtual ~PropertiesConnectorBase();

    virtual void Save()=0;
    virtual void Restore()=0;
private:
    void update();

protected:
    friend class PropertiesConnectorsContainer;
    Setter _setter;
    PropertyPtr _propertyPtr;
    QMetaObject::Connection _connection;
};

class _Export PropertiesCheckBoxConnector : public PropertiesConnectorBase
{
    bool _oldValue;
public:
    PropertiesCheckBoxConnector(const Name& propertyName, QCheckBox* checkBox);

    virtual void Save() Q_DECL_OVERRIDE;
    virtual void Restore() Q_DECL_OVERRIDE;
};

class _Export PropertiesLineEditConnector : public PropertiesConnectorBase
{
    QString _oldValue;
public:
    PropertiesLineEditConnector(const Name& propertyName, QLineEdit* lineEdit);

    virtual void Save() Q_DECL_OVERRIDE;
    virtual void Restore() Q_DECL_OVERRIDE;
};

template<class SpinType, typename valueType>
class PropertiesSpinBoxConnectorBase : public PropertiesConnectorBase
{
    valueType _oldValue;
public:
    PropertiesSpinBoxConnectorBase(const Name& propertyName, SpinType* spinBox)
        : PropertiesConnectorBase(propertyName,
                                  [spinBox, this](const QVariant& value){
                                    auto property = _propertyPtr.GetProperty();
                                    spinBox->setMinimum(property->GetMin().toDouble());
                                    spinBox->setMaximum(property->GetMax().toDouble());
                                    spinBox->setValue(value.toDouble());
                                    auto singleStep = (spinBox->maximum() - spinBox->minimum()) / 100.0;
                                    singleStep = (singleStep > 1.0) ? 1.0 : singleStep;
                                    spinBox->setSingleStep(singleStep);
                                    spinBox->setFocusPolicy(Qt::StrongFocus);
                                  },
                                  spinBox)
    {
        _connection = connect(spinBox, static_cast<void (SpinType::*)(valueType)>(&SpinType::valueChanged), [this](valueType value){
            _propertyPtr.GetProperty()->SetValue(value);
        });
    }

    virtual void Save() Q_DECL_OVERRIDE { _oldValue = reinterpret_cast<SpinType*>(parent())->value(); }
    virtual void Restore() Q_DECL_OVERRIDE { reinterpret_cast<SpinType*>(parent())->setValue(_oldValue); }
};

typedef PropertiesSpinBoxConnectorBase<QSpinBox, qint32> PropertiesSpinBoxConnector;
typedef PropertiesSpinBoxConnectorBase<QDoubleSpinBox, double> PropertiesDoubleSpinBoxConnector;


#endif // PROPERTIESWIDGETSCONNECTOR_H
