#include "localpropertieswidgetconnector.h"

#ifdef QT_GUI_LIB

#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QRadioButton>

LocalPropertiesWidgetConnectorBase::LocalPropertiesWidgetConnectorBase(const Setter& widgetSetter, const Setter& propertySetter)
    : m_widgetSetter([this, widgetSetter](){
        if(!m_ignorePropertyChange) {
            guards::LambdaGuard guard([this]{ m_ignoreWidgetChange = false; }, [this] { m_ignoreWidgetChange = true; } );
            widgetSetter();
        }
    })
    , m_propertySetter([this, propertySetter]{
        if(!m_ignoreWidgetChange) {
            guards::LambdaGuard guard([this]{ m_ignorePropertyChange = false; }, [this] { m_ignorePropertyChange = true; } );
            propertySetter();
        }
    })
    , m_dispatcherConnections(this)
    , m_ignorePropertyChange(false)
    , m_ignoreWidgetChange(false)
{
    m_widgetSetter();
}

LocalPropertiesCheckBoxConnector::LocalPropertiesCheckBoxConnector(LocalProperty<bool>* property, QCheckBox* checkBox)
    : Super([checkBox, property]{
                checkBox->setChecked(*property);
            },
            [property, checkBox]{
                *property = checkBox->isChecked();
            }
    )
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });

    m_connections.connect(checkBox, &QCheckBox::clicked, [this](bool value){
        m_propertySetter();
    });
}


LocalPropertiesLineEditConnector::LocalPropertiesLineEditConnector(LocalProperty<QString>* property, QLineEdit* lineEdit)
    : Super([lineEdit, property](){
               lineEdit->setText(*property);
            },
            [lineEdit, property](){
               *property = lineEdit->text();
            }
    )
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });

    m_connections.connect(lineEdit, &QLineEdit::editingFinished, [this](){
        m_propertySetter();
    });
}

LocalPropertiesTextEditConnector::LocalPropertiesTextEditConnector(LocalProperty<QString>* property, QTextEdit* textEdit, LocalPropertiesTextEditConnector::SubmitType submitType)
    : Super([textEdit, property](){
               textEdit->setText(*property);
            }, [textEdit, property]{
               *property = textEdit->toPlainText();
            })
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });

    switch (submitType) {
    case SubmitType_OnEveryChange:
        m_connections.connect(textEdit, &QTextEdit::textChanged, [this](){
            m_propertySetter();
        });
        break;
    default:
        break;
    }
}

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, QDoubleSpinBox* spinBox)
    : Super([spinBox, property](){
                spinBox->setRange(property->GetMin(), property->GetMax());
                spinBox->setValue(*property);
            },
            [spinBox, property](){
                *property = spinBox->value();
            }
    )
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });
    m_dispatcherConnections.Add(property->OnMinMaxChanged,[spinBox, property]{
        QSignalBlocker blocker(spinBox);
        spinBox->setRange(property->GetMin(), property->GetMax());
    });

    m_connections.connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, spinBox](){
        m_propertySetter();
    });
}

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, QDoubleSpinBox* spinBox)
    : Super([spinBox, property](){
                spinBox->setRange(property->GetMin(), property->GetMax());
                spinBox->setValue(*property);
            },
            [spinBox, property](){
                *property = spinBox->value();
            }
    )
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });
    m_dispatcherConnections.Add(property->OnMinMaxChanged,[spinBox, property]{
        QSignalBlocker blocker(spinBox);
        spinBox->setRange(property->GetMin(), property->GetMax());
    });

    m_connections.connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](){
        m_propertySetter();
    });
}

LocalPropertiesSpinBoxConnector::LocalPropertiesSpinBoxConnector(LocalPropertyInt* property, QSpinBox* spinBox)
    : Super([spinBox, property](){
                spinBox->setRange(property->GetMin(), property->GetMax());
                spinBox->setValue(*property);
            },
            [spinBox, property](){
                *property = spinBox->value();
            }
    )
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });
    m_dispatcherConnections.Add(property->OnMinMaxChanged,[spinBox, property]{
        QSignalBlocker blocker(spinBox);
        spinBox->setRange(property->GetMin(), property->GetMax());
    });

    m_connections.connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](){
        m_propertySetter();
    });
}

void LocalPropertiesWidgetConnectorsContainer::Clear()
{
    m_connectors.Clear();
}

LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalPropertyNamedUint* property, QComboBox* comboBox)
    : Super([property, comboBox]{
                comboBox->setCurrentIndex(*property);
            },
            [property, comboBox]{
                *property = comboBox->currentIndex();
            }
    )
{
    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });

    m_connections.connect(comboBox, static_cast<void (QComboBox::*)(qint32)>(&QComboBox::currentIndexChanged), [this]{
        m_propertySetter();
    });
}

LocalPropertiesRadioButtonsConnector::LocalPropertiesRadioButtonsConnector(LocalPropertyNamedUint* property, const Stack<QRadioButton*>& buttons)
    : Super([property, buttons]{
                buttons[*property]->setChecked(true);
                qint32 i(0); // In case if we don't use a GroupBox
                for(auto* button : buttons) {
                    if(i != *property && button->isChecked()) {
                        button->setChecked(false);
                    }
                }
            },
            [property, this]{
                *property = m_currentIndex;
            }
    )
    , m_currentIndex(*property)
{
    Q_ASSERT(!buttons.IsEmpty());

    m_dispatcherConnections.Add(property->GetDispatcher(),[this]{
        m_widgetSetter();
    });

    qint32 i = 0;
    for(auto* button : buttons) {
        m_connections.connect(button, &QRadioButton::clicked, [this, i]{
            m_currentIndex = i;
            m_propertySetter();
        });
        i++;
    }
}

#endif
