#include "localpropertieswidgetconnector.h"

#ifdef QT_GUI_LIB

#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>
#include <QDateTimeEdit>

LocalPropertiesWidgetConnectorBase::LocalPropertiesWidgetConnectorBase(const Setter& widgetSetter, const Setter& propertySetter)
    : m_widgetSetter([this, widgetSetter](){
        if(!m_ignorePropertyChange) {
            guards::LambdaGuard guard([this]{ m_ignoreWidgetChange = false; }, [this] { m_ignoreWidgetChange = true; } );
            widgetSetter();
        }
    })
    , m_propertySetter([this, propertySetter, widgetSetter]{
        if(!m_ignoreWidgetChange) {
            guards::LambdaGuard guard([this]{ m_ignorePropertyChange = false; }, [this] { m_ignorePropertyChange = true; } );
            propertySetter();
            guards::LambdaGuard guard2([this]{ m_ignoreWidgetChange = false; }, [this] { m_ignoreWidgetChange = true; } );
            widgetSetter();
        }
    })
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
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(checkBox, &QCheckBox::clicked, [this](bool){
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
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(lineEdit, &QLineEdit::editingFinished, [this](){
        m_propertySetter();
    });
}

LocalPropertiesLabelConnector::LocalPropertiesLabelConnector(LocalPropertyString* property, class QLabel* label)
    : Super([label, property]{
        label->setText(*property);
    }, []{})
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesTextEditConnector::LocalPropertiesTextEditConnector(LocalProperty<QString>* property, QTextEdit* textEdit, LocalPropertiesTextEditConnector::SubmitType submitType)
    : Super([textEdit, property](){
               textEdit->setText(*property);
            }, [textEdit, property]{
               *property = textEdit->toPlainText();
            })
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

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
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    property->OnMinMaxChanged.Connect(this, [spinBox, property]{
        QSignalBlocker blocker(spinBox);
        spinBox->setRange(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](){
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
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    property->OnMinMaxChanged.Connect(this, [spinBox, property]{
        QSignalBlocker blocker(spinBox);
        spinBox->setRange(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);

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
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    property->OnMinMaxChanged.Connect(this, [spinBox, property]{
        QSignalBlocker blocker(spinBox);
        spinBox->setRange(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](){
        m_propertySetter();
    });
}

void LocalPropertiesWidgetConnectorsContainer::Clear()
{
    m_connectors.Clear();
}

LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalPropertyInt* property, QComboBox* comboBox)
    : Super([property, comboBox]{
                comboBox->setCurrentIndex(*property);
            },
            [property, comboBox]{
                *property = comboBox->currentIndex();
            }
    )
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(comboBox, static_cast<void (QComboBox::*)(qint32)>(&QComboBox::currentIndexChanged), [this]{
        m_propertySetter();
    });
}

LocalPropertiesRadioButtonsConnector::LocalPropertiesRadioButtonsConnector(LocalPropertyInt* property, const Stack<QRadioButton*>& buttons)
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

    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    qint32 i = 0;
    for(auto* button : buttons) {
        m_connections.connect(button, &QRadioButton::clicked, [this, i]{
            m_currentIndex = i;
            m_propertySetter();
        });
        i++;
    }
}

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalProperty<QDate> * property, QDateEdit * dateTime)
    : Super([dateTime, property](){
                dateTime->setDate(*property);
            },
            [dateTime, property](){
                *property = dateTime->date();
            }
    )
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(dateTime, &QDateEdit::dateChanged, [this](){
        m_propertySetter();
    });
}

LocalPropertiesDateTimeConnector::LocalPropertiesDateTimeConnector(LocalProperty<QDateTime>* property, QDateTimeEdit* dateTime)
    : Super([dateTime, property](){
                dateTime->setDateTime(*property);
            },
            [dateTime, property](){
                *property = dateTime->dateTime();
            }
    )
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(dateTime, &QDateTimeEdit::dateTimeChanged, [this](){
        m_propertySetter();
    });
}
#endif
