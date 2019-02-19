#include "propertieswidgetsconnector.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>

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

PropertiesConnectorBase::PropertiesConnectorBase(const Name& name, const PropertiesConnectorBase::Setter& setter, QWidget* target)
    : QObject(target)
    , _setter(setter)
    , _propertyPtr(name, [this, target]{
    Q_ASSERT(_propertyPtr.GetProperty()->GetOptions().TestFlag(Property::Option_IsPresentable));
    if(!_ignorePropertyChange) {
        QSignalBlocker blocker(target);
        _setter(_propertyPtr.GetProperty()->GetValue());
    }
}, PropertiesConnectorContextIndexGuard::currentContextIndex())
    , _ignorePropertyChange(false)
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

PropertiesCheckBoxConnector::PropertiesCheckBoxConnector(const Name& propertyName, QCheckBox* checkBox)
    : PropertiesConnectorBase(propertyName,
                              [checkBox](const QVariant& value){ checkBox->setChecked(value.toBool()); },
                              checkBox)
{
    _connection = connect(checkBox, &QCheckBox::clicked, [this](bool value){
        PropertyChangeGuard guard(this);
        _propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesLineEditConnector::PropertiesLineEditConnector(const Name& propertyName, QLineEdit* lineEdit)
    : PropertiesConnectorBase(propertyName,
                              [lineEdit](const QVariant& value){ lineEdit->setText(value.toString()); },
                              lineEdit)
{
    _connection = connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit](){
        PropertyChangeGuard guard(this);
        _propertyPtr.GetProperty()->SetValue(lineEdit->text());
    });
}

PropertiesSpinBoxConnector::PropertiesSpinBoxConnector(const Name& propertyName, QSpinBox* spinBox)
    : PropertiesConnectorBase(propertyName,
                              [spinBox, this](const QVariant& value){
                                  auto property = _propertyPtr.GetProperty();
                                  spinBox->setMinimum(property->GetMin().toDouble());
                                  spinBox->setMaximum(property->GetMax().toDouble());
                                  spinBox->setValue(value.toDouble());
                                  spinBox->setSingleStep(1);
                                  spinBox->setFocusPolicy(Qt::StrongFocus);
                              },
                              spinBox)
{
    _connection = connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int value){
        PropertyChangeGuard guard(this);
        _propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesDoubleSpinBoxConnector::PropertiesDoubleSpinBoxConnector(const Name& propertyName, QDoubleSpinBox* spinBox)
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
    _connection = connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double value){
        PropertyChangeGuard guard(this);
        _propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesTextEditConnector::PropertiesTextEditConnector(const Name& propertyName, QTextEdit* textEdit)
    : PropertiesConnectorBase(propertyName,
                              [textEdit, this](const QVariant& value){ textEdit->setText(value.toString()); },
                              textEdit)
{
    _connection = connect(textEdit, &QTextEdit::textChanged, [this, textEdit](){
        PropertyChangeGuard guard(this);
        _propertyPtr.GetProperty()->SetValue(textEdit->toPlainText());
    });
}

PropertiesGroupBoxConnector::PropertiesGroupBoxConnector(const Name& propertyName, QGroupBox* groupBox)
    : PropertiesConnectorBase(propertyName,
                              [groupBox](const QVariant& value){ groupBox->setChecked(value.toBool()); },
                              groupBox)
{
    _connection = connect(groupBox, &QGroupBox::clicked, [this](bool value){
        PropertyChangeGuard guard(this);
        _propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesConnectorBase::PropertyChangeGuard::PropertyChangeGuard(PropertiesConnectorBase* connector)
    : _ignorePropertyChange(connector->_ignorePropertyChange)
{
    _ignorePropertyChange = true;
}

PropertiesConnectorBase::PropertyChangeGuard::~PropertyChangeGuard()
{
    _ignorePropertyChange = false;
}
