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
#include <QMenu>
#include <QPushButton>

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

LocalPropertiesMenuLabelConnector::LocalPropertiesMenuLabelConnector(LocalPropertyString* property, QMenu* menu)
    : Super([menu, property]{
        menu->setTitle(*property);
    }, []{})
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
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


LocalPropertiesLineEditConnector::LocalPropertiesLineEditConnector(LocalProperty<QString>* property, QLineEdit* lineEdit, bool reactive)
    : Super([lineEdit, property](){
               if(lineEdit->text() != *property) {
                   lineEdit->setText(*property);
               }
            },
            [lineEdit, property](){
               *property = lineEdit->text();
            }
    ), m_textChanged(250)
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    if(reactive){
        m_connections.connect(lineEdit, &QLineEdit::textChanged, [this]{
            if(!m_ignoreWidgetChange) {
                m_textChanged.Call([this]{ m_propertySetter(); });
            }
        });
    } else {
        m_connections.connect(lineEdit, &QLineEdit::editingFinished, [this](){
            m_propertySetter();
        });
    }
}

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(Dispatcher* dispatcher, QPushButton* button)
    : Super([]{}, [dispatcher]{ dispatcher->Invoke(); })
{
    m_connections.connect(button, &QPushButton::clicked, [this](){
        m_propertySetter();
    });
}

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(LocalPropertyBool* checkedProperty, QPushButton* button)
    : Super([button,checkedProperty]{
                button->setChecked(*checkedProperty);
            }, [button,checkedProperty]{
                *checkedProperty = button->isChecked();
            })
{
    Q_ASSERT(button->isCheckable());

    checkedProperty->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(button, &QPushButton::clicked, [this](){
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
               if(textEdit->toPlainText() != *property){
                   textEdit->setText(*property);
               }
            }, [textEdit, property]{
               *property = textEdit->toPlainText();
            }), m_textChanged(250)
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    switch (submitType) {
    case SubmitType_OnEveryChange:
        m_connections.connect(textEdit, &QTextEdit::textChanged, [this](){
            if(!m_ignoreWidgetChange){
                m_textChanged.Call([this]{ m_propertySetter();});
            }
        });
        break;
    default:
        break;
    }
}

#ifdef WIDGETS_MODULE_LIB

#include <WidgetsModule/internal.hpp>

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double presicion)
    : LocalPropertiesDoubleSpinBoxConnector(&property->Value, spinBox, presicion)
{
    spinBox->MakeOptional(&property->IsValid).MakeSafe(m_dispatcherConnections);
}
#endif

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, QDoubleSpinBox* spinBox, double presicion)
    : Super([spinBox, property, presicion](){
                if(!fuzzyCompare(spinBox->minimum(), property->GetMin(), presicion) || !fuzzyCompare(spinBox->maximum(), property->GetMax(), presicion)) {
                    spinBox->setRange(property->GetMin(), property->GetMax());
                }
                if(!fuzzyCompare(spinBox->value(), *property, presicion)) {
                    spinBox->setValue(*property);
                }
            },
            [spinBox, property](){
                property->SetValue(spinBox->value());
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

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, QDoubleSpinBox* spinBox, float presicion)
    : Super([spinBox, property, presicion](){
                if(!fuzzyCompare((float)spinBox->minimum(), property->GetMin(), presicion) || !fuzzyCompare((float)spinBox->maximum(), property->GetMax(), presicion)) {
                    spinBox->setRange(property->GetMin(), property->GetMax());
                }
                if(!fuzzyCompare((float)spinBox->value(), *property, presicion)) {
                    spinBox->setValue(*property);
                }
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
                if(spinBox->minimum() != property->GetMin() || spinBox->maximum() != property->GetMax()) {
                    spinBox->setRange(property->GetMin(), property->GetMax());
                }
                if(spinBox->value() != *property) {
                    spinBox->setValue(*property);
                }
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
    onClear();
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

    m_connections.connect(comboBox, static_cast<void (QComboBox::*)(qint32)>(&QComboBox::activated), [this]{
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
                    i++;
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

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDate* property, WidgetsDateEdit* dateTime)
    : Super([](){},
            [](){}
    )
{
    property->ConnectBoth(dateTime->CurrentDate, [](const QDate& dt){ return dt; }, [](const QDate& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(this, [property, dateTime]{
        dateTime->CurrentDate.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDateTime* property, WidgetsDateEdit* dateTime)
    : Super([](){},
            [](){}
    )
{
    property->ConnectBoth(dateTime->CurrentDateTime, [](const QDateTime& dt){ return dt; }, [](const QDateTime& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(this, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDateTimeConnector::LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, WidgetsDateTimeEdit* dateTime)
    : Super([](){},
            [](){}
    )
{
    property->ConnectBoth(dateTime->CurrentDateTime, [](const QDateTime& dt){ return dt; }, [](const QDateTime& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(this, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

#endif
