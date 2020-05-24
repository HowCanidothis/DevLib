#include "propertieswidgetsconnector.h"

#ifdef QT_GUI_LIB

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>

PropertiesConnectorBase::PropertiesConnectorBase(const Name& name, const PropertiesConnectorBase::Setter& setter, QWidget* target)
    : m_setter(setter)
    , m_ignorePropertyChange(false)
    , m_dispatcherConnections(this)
    , m_target(target)
    , m_propertyName(name)
{
}

PropertiesConnectorBase::~PropertiesConnectorBase()
{
    disconnect(m_connection);
}

void PropertiesConnectorBase::SetScope(const PropertiesScopeName& scope)
{
    m_dispatcherConnections.Clear();
    m_propertyPtr.Assign(m_propertyName, scope);
    m_dispatcherConnections.Add(m_propertyPtr.GetDispatcher(), [this]{
        Q_ASSERT(m_propertyPtr.IsValid());
        Q_ASSERT(m_propertyPtr.GetProperty()->GetOptions().TestFlag(Property::Option_IsPresentable));
        if(!m_ignorePropertyChange) {
            QSignalBlocker blocker(m_target);
            m_setter(m_propertyPtr.GetProperty()->GetValue());
        }
    });

    if(m_propertyPtr.IsValid()) {
        QSignalBlocker blocker(m_target);
        m_setter(m_propertyPtr.GetProperty()->GetValue());
    }
}

void PropertiesConnectorBase::Update()
{
    QSignalBlocker blocker(parent());
    m_setter(m_propertyPtr.GetProperty()->GetValue());
}

void PropertiesConnectorsContainer::SetScope(const PropertiesScopeName& scope)
{
    for(auto connector : m_connectors) {
        connector->SetScope(scope);
    }
}

void PropertiesConnectorsContainer::Update()
{
    for(auto connector : m_connectors) {
        connector->Update();
    }
}

void PropertiesConnectorsContainer::Clear()
{
    m_connectors.Clear();
}

PropertiesCheckBoxConnector::PropertiesCheckBoxConnector(const Name& propertyName, QCheckBox* checkBox)
    : PropertiesConnectorBase(propertyName,
                              [checkBox](const QVariant& value){ checkBox->setChecked(value.toBool()); },
                              checkBox)
{
    m_connection = connect(checkBox, &QCheckBox::clicked, [this](bool value){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesLineEditConnector::PropertiesLineEditConnector(const Name& propertyName, QLineEdit* lineEdit)
    : PropertiesConnectorBase(propertyName,
                              [lineEdit](const QVariant& value){ lineEdit->setText(value.toString()); },
                              lineEdit)
{
    m_connection = connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit](){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(lineEdit->text());
    });
}

PropertiesSpinBoxConnector::PropertiesSpinBoxConnector(const Name& propertyName, QSpinBox* spinBox)
    : PropertiesConnectorBase(propertyName,
                              [spinBox, this](const QVariant& value){
                                  auto property = m_propertyPtr.GetProperty();
                                  spinBox->setMinimum(property->GetMin().toDouble());
                                  spinBox->setMaximum(property->GetMax().toDouble());
                                  spinBox->setValue(value.toDouble());
                                  spinBox->setSingleStep(1);
                                  spinBox->setFocusPolicy(Qt::StrongFocus);
                              },
                              spinBox)
{
    m_connection = connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int value){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesDoubleSpinBoxConnector::PropertiesDoubleSpinBoxConnector(const Name& propertyName, QDoubleSpinBox* spinBox)
    : PropertiesConnectorBase(propertyName,
                              [spinBox, this](const QVariant& value){
                                  auto property = m_propertyPtr.GetProperty();
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
    m_connection = connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double value){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesTextEditConnector::PropertiesTextEditConnector(const Name& propertyName, QTextEdit* textEdit, SubmitType submitType)
    : PropertiesConnectorBase(propertyName,
                              [textEdit](const QVariant& value){ textEdit->setText(value.toString()); },
                              textEdit)
{
    switch (submitType) {
    case SubmitType_OnEveryChange:
        m_connection = connect(textEdit, &QTextEdit::textChanged, [this, textEdit](){
            PropertyChangeGuard guard(this);
            m_propertyPtr.GetProperty()->SetValue(textEdit->toPlainText());
        });
        break;
    default:
        break;
    }
}

PropertiesGroupBoxConnector::PropertiesGroupBoxConnector(const Name& propertyName, QGroupBox* groupBox)
    : PropertiesConnectorBase(propertyName,
                              [groupBox](const QVariant& value){ groupBox->setChecked(value.toBool()); },
                              groupBox)
{
    m_connection = connect(groupBox, &QGroupBox::clicked, [this](bool value){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(value);
    });
}

PropertiesConnectorBase::PropertyChangeGuard::PropertyChangeGuard(PropertiesConnectorBase* connector)
    : m_ignorePropertyChange(connector->m_ignorePropertyChange)
{
    m_ignorePropertyChange = true;
}

PropertiesConnectorBase::PropertyChangeGuard::~PropertyChangeGuard()
{
    m_ignorePropertyChange = false;
}

PropertiesRadioButtonsGroupBoxConnector::PropertiesRadioButtonsGroupBoxConnector(const Name& propertyName, QGroupBox* groupBox, const Stack<QRadioButton*>& buttons)
    : PropertiesConnectorBase(propertyName,
                              [groupBox, buttons](const QVariant& value){
                                    auto index = value.toUInt();
                                    Q_ASSERT(index >= 0 && index < buttons.Size());
                                    auto* targetButton = buttons.At(index);
                                    QSignalBlocker signalBlocker(targetButton);
                                    targetButton->setChecked(true);
                              },
                              groupBox)
{


    Q_ASSERT(!buttons.IsEmpty());
    m_connection = connect(this, &PropertiesRadioButtonsGroupBoxConnector::valueChanged, [this](qint32 value){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(value);
    });

    qint32 i = 0;
    for(auto* button : buttons) {
        connect(button, &QRadioButton::clicked, [this, i]{
            emit valueChanged(i);
        });
        i++;
    }
}

Stack<QRadioButton*> PropertiesRadioButtonsGroupBoxConnector::ButtonsFromGroup(QGroupBox* groupBox)
{
    Stack<QRadioButton*> result;
    for(auto* child : groupBox->children()) {
        if(auto* radioButton = qobject_cast<QRadioButton*>(child)) {
            result.Append(radioButton);
        }
    }
    return result;
}

PropertiesComboBoxConnector::PropertiesComboBoxConnector(const Name& propertyName, QComboBox* comboBox)
    : PropertiesConnectorBase(propertyName,
                              [comboBox, this](const QVariant& value){
                                  comboBox->setCurrentIndex(value.toUInt());
                              },
                              comboBox)
{
    m_connection = connect(comboBox, static_cast<void (QComboBox::*)(qint32)>(&QComboBox::currentIndexChanged), [this](qint32 value){
        PropertyChangeGuard guard(this);
        m_propertyPtr.GetProperty()->SetValue(value);
    });
}

#endif
