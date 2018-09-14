#include "propertieswidgetsconnector.h"
#include <QCheckBox>
#include <QLineEdit>

PropertiesConnectorContextIndexGuard::PropertiesConnectorContextIndexGuard(properties_context_index_t contextIndex)
    : _before(currentContextIndex())
{
    currentContextIndex() = contextIndex;
}

PropertiesConnectorContextIndexGuard::PropertiesConnectorContextIndexGuard()
    : _before(currentContextIndex())
{
    currentContextIndex() = PropertiesSystem::GetCurrentContextIndex();
}

PropertiesConnectorContextIndexGuard::~PropertiesConnectorContextIndexGuard()
{
    currentContextIndex() = _before;
}

properties_context_index_t& PropertiesConnectorContextIndexGuard::currentContextIndex()
{
    static properties_context_index_t ret = PropertiesSystem::Global;
    return ret;
}

PropertiesConnectorBase::PropertiesConnectorBase(const Name& name, const PropertiesConnectorBase::Setter& setter, QObject* target)
    : QObject(target)
    , _setter(setter)
    , _propertyPtr(name, [this, target]{
    QSignalBlocker blocker(target);
    _setter(_propertyPtr.GetProperty()->GetValue());
}, PropertiesConnectorContextIndexGuard::currentContextIndex())
{
    if(_propertyPtr.IsValid()) {
        QSignalBlocker blocker(target);
        _setter(_propertyPtr.GetProperty()->GetValue());
    }
}

PropertiesConnectorBase::~PropertiesConnectorBase()
{
    disconnect(_connection);
}

void PropertiesConnectorBase::update()
{
    QSignalBlocker blocker(parent());
    _setter(_propertyPtr.GetProperty()->GetValue());
}

void PropertiesConnectorsContainer::AddConnector(PropertiesConnectorBase* connector)
{
    _connectors.Append(connector);
}

void PropertiesConnectorsContainer::Update()
{
    for(auto connector : _connectors) {
        connector->update();
    }
}

void PropertiesConnectorsContainer::Clear()
{
    _connectors.Clear();
}

void PropertiesConnectorsContainer::Save()
{
    for(auto connector : _connectors) {
        connector->Save();
    }
}

void PropertiesConnectorsContainer::Restore()
{
    for(auto connector : _connectors) {
        connector->Restore();
    }
}


PropertiesCheckBoxConnector::PropertiesCheckBoxConnector(const Name& propertyName, QCheckBox* checkBox)
    : PropertiesConnectorBase(propertyName,
                              [checkBox](const QVariant& value){ checkBox->setChecked(value.toBool()); },
                              checkBox)
{
    _connection = connect(checkBox, &QCheckBox::clicked, [this](bool value){
        _propertyPtr.GetProperty()->SetValue(value);
    });
}

void PropertiesCheckBoxConnector::Save()
{
    _oldValue = reinterpret_cast<QCheckBox*>(parent())->isChecked();
}

void PropertiesCheckBoxConnector::Restore()
{
    reinterpret_cast<QCheckBox*>(parent())->setChecked(_oldValue);
}

PropertiesLineEditConnector::PropertiesLineEditConnector(const Name& propertyName, QLineEdit* lineEdit)
    : PropertiesConnectorBase(propertyName,
                              [lineEdit](const QVariant& value){ lineEdit->setText(value.toString()); },
                              lineEdit)
{
    _connection = connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit](){
        _propertyPtr.GetProperty()->SetValue(lineEdit->text());
    });
}

void PropertiesLineEditConnector::Save()
{
    _oldValue = reinterpret_cast<QLineEdit*>(parent())->text();
}

void PropertiesLineEditConnector::Restore()
{
    reinterpret_cast<QLineEdit*>(parent())->setText(_oldValue);
}
