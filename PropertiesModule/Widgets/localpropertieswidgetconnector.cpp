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

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(const QVector<QPushButton*>& buttons, LocalPropertyInt* property)
    : Super([buttons, property]{
                for(auto* button : buttons) {
                    button->setChecked(false);
                }
                if(*property >= 0 && *property < buttons.size()) {
                    buttons.at(*property)->setChecked(true);
                }
            }, [this, property]{
                *property = m_currentIndex;
            })
    , m_currentIndex(*property)
{

    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    qint32 i(0);
    for(auto* button : buttons) {
        button->setCheckable(true);
        m_connections.connect(button, &QPushButton::clicked, [i, this]{
            m_currentIndex = i;
            m_propertySetter();
        });
        ++i;
    }

    auto resetCheckState = [buttons]{
        for(auto* button : buttons) {
            button->setChecked(false);
        }
    };
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

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox)
    : LocalPropertiesDoubleSpinBoxConnector(&property->Value, spinBox, [property](double value){
        property->Value = value;
    })
{
    spinBox->MakeOptional(&property->IsValid).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesSpinBoxConnector::LocalPropertiesSpinBoxConnector(LocalPropertyIntOptional* property, WidgetsSpinBoxWithCustomDisplay* spinBox)
    : LocalPropertiesSpinBoxConnector(&property->Value, spinBox)
{
    spinBox->MakeOptional(&property->IsValid).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDoubleDisplay* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox)
    : LocalPropertiesDoubleSpinBoxConnector(&property->DisplayValue, spinBox,[](double){})
{
    property->Precision.OnChanged.Connect(this, [spinBox, property]{
        spinBox->setDecimals(property->Precision);
    }).MakeSafe(m_dispatcherConnections);
}

#endif

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, QDoubleSpinBox* spinBox, const std::function<void (double)>& propertySetter)
    : Super([spinBox, property](){
                auto precision = epsilon(spinBox->decimals() + 1);
                if(!fuzzyCompare(spinBox->minimum(), property->GetMin(), precision) || !fuzzyCompare(spinBox->maximum(), property->GetMax(), precision)) {
                    spinBox->setRange(property->GetMin(), property->GetMax());
                }
                if(!fuzzyCompare(spinBox->value(), *property, precision)) {
                    spinBox->setValue(*property);
                }
            },
            [spinBox, property, propertySetter](){
                if(propertySetter != nullptr) {
                    propertySetter(spinBox->value());
                    return;
                }
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
    
    m_connections.connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](){
        m_propertySetter();
    });
}

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyFloat* property, QDoubleSpinBox* spinBox)
    : Super([spinBox, property](){
                float precision = epsilon(spinBox->decimals());
                if(!fuzzyCompare((float)spinBox->minimum(), property->GetMin(), precision) || !fuzzyCompare((float)spinBox->maximum(), property->GetMax(), precision)) {
                    spinBox->setRange(property->GetMin(), property->GetMax());
                }
                if(!fuzzyCompare((float)spinBox->value(), *property, precision)) {
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

    m_connections.connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](){
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
    
    m_connections.connect(spinBox, QOverload<qint32>::of(&QSpinBox::valueChanged), [this](){
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

    connectComboBox(comboBox);
}

void LocalPropertiesComboBoxConnector::connectComboBox(QComboBox* comboBox)
{
    m_connections.connect(comboBox, QOverload<qint32>::of(&QComboBox::activated), [this]{
        m_propertySetter();
    });

    m_connections.connect(comboBox->model(), &QAbstractItemModel::modelReset, [this]{
        m_widgetSetter();
    });
}

LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalProperty<Name>* property, QComboBox* comboBox)
    : Super([property, comboBox]{
                qint32 result = 0;
                forEachModelIndex(comboBox->model(), QModelIndex(), [&result, property](const QModelIndex& index){
                    if(index.data(Qt::EditRole).value<Name>() == *property) {
                        return true;
                    }
                    result++;
                    return false;
                });
                comboBox->setCurrentIndex(result);
            },
            [property, comboBox]{
                *property = comboBox->currentData(Qt::EditRole).value<Name>();
            }
    )
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    connectComboBox(comboBox);
}

#ifdef WIDGETS_MODULE_LIB
LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalPropertyInt* property, QComboBox* comboBox, const ModelsStandardListModelPtr& model)
    : Super([property, comboBox, model]{
                qint32 result = 0;
                forEachModelIndex(comboBox->model(), QModelIndex(), [&result, property](const QModelIndex& index){
                    if(index.data(Qt::EditRole).toInt() == *property) {
                        return true;
                    }
                    result++;
                    return false;
                });
                comboBox->setCurrentIndex(result);
            },
            [property, comboBox]{
                *property = comboBox->currentData(Qt::EditRole).toInt();
            }
    )
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    model->OnChanged.Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    connectComboBox(comboBox);
}

LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalProperty<Name>* property, QComboBox* comboBox, const SharedPointer<ModelsStandardListModel>& model)
    : LocalPropertiesComboBoxConnector(property, comboBox)
{
    model->OnChanged.Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
}
#endif

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

#ifdef WIDGETS_MODULE_LIB
LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDate* property, WidgetsDateEdit* dateTime)
    : Super([](){},
            [](){}
    )
{
    property->ConnectBoth(CONNECTION_DEBUG_LOCATION, dateTime->CurrentDate, [](const QDate& dt){ return dt; }, [](const QDate& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(this, [property, dateTime]{
        dateTime->CurrentDate.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDateTime* property, WidgetsDateEdit* dateTime)
    : Super([](){},
            [](){}
    )
{
    property->ConnectBoth(CONNECTION_DEBUG_LOCATION, dateTime->CurrentDateTime, [](const QDateTime& dt){ return dt; }, [](const QDateTime& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(this, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDateTimeConnector::LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, WidgetsDateTimeEdit* dateTime)
    : Super([](){},
            [](){}
    )
{
    property->ConnectBoth(CONNECTION_DEBUG_LOCATION, dateTime->CurrentDateTime, [](const QDateTime& dt){ return dt; }, [](const QDateTime& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(this, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

#endif

#endif
