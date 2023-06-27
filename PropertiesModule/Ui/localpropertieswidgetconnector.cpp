#include "localpropertieswidgetconnector.h"

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
#include <QAbstractButton>>

#include <WidgetsModule/internal.hpp>

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
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesCheckBoxConnector::LocalPropertiesCheckBoxConnector(LocalPropertyBool* property, QCheckBox* checkBox)
    : Super([checkBox, property]{
                WidgetCheckBoxWrapper(checkBox).WidgetChecked() = property->Native();
            },
            [property, checkBox]{
                *property = checkBox->isChecked();
            }
    )
{
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    WidgetCheckBoxWrapper(checkBox).WidgetChecked().OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_propertySetter();
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesCheckBoxConnector::LocalPropertiesCheckBoxConnector(LocalPropertyString* property, QCheckBox* checkBox)
    : Super([checkBox, property]{
                WidgetCheckBoxWrapper(checkBox).WidgetText()->SetTranslationHandler(TR(property->Native(), property));
            },
            [property, checkBox]{
                *property = checkBox->text();
            }
    )
{
    property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this](){
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    WidgetCheckBoxWrapper(checkBox).WidgetText()->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_propertySetter();
    }).MakeSafe(m_dispatcherConnections);
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
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    if(reactive){
        m_connections.connect(lineEdit, &QLineEdit::textChanged, [this]{
            if(!m_ignoreWidgetChange) {
                m_textChanged.Call(CONNECTION_DEBUG_LOCATION, [this]{ m_propertySetter(); });
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

    checkedProperty->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(button, &QPushButton::clicked, [this](){
        m_propertySetter();
    });
}

template <typename T1, typename T2>
QVector<T2> mutate(const QVector<T1>& source){
    QVector<T2> result; result.reserve(source.size());
    for(const auto& value : source){
        result.append(value);
    }
    return result;
}

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(LocalPropertyInt* property, const QVector<QPushButton*>& buttons)
    : LocalPropertiesPushButtonConnector(property, mutate<QPushButton*, QAbstractButton*>(buttons))
{

}

LocalPropertiesLabelConnector::LocalPropertiesLabelConnector(LocalPropertyString* property, class QLabel* label)
    : Super([label, property]{
    label->setText(*property);
}, []{})
{
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
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
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    switch (submitType) {
    case SubmitType_OnEveryChange:
        m_connections.connect(textEdit, &QTextEdit::textChanged, [this](){
            if(!m_ignoreWidgetChange){
                m_textChanged.Call(CONNECTION_DEBUG_LOCATION, [this]{ m_propertySetter();});
            }
        });
        break;
    default:
        break;
    }
}

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(LocalPropertyInt* property, const QVector<QAbstractButton*>& buttons)
    : Super([buttons, property]{
                for(auto* button : buttons) {
                    WidgetAbstractButtonWrapper wrapper (button);
                    auto& checked = wrapper.WidgetChecked();
                    checked = false;
                    if(wrapper.GetWidget()->isChecked() != checked) { // Qt internally changed value
                        checked.Invoke();
                    }
                }
                if(*property >= 0 && *property < buttons.size()) {
                    WidgetAbstractButtonWrapper(buttons.at(*property)).WidgetChecked() = true;
                }
            }, [this, property]{
                *property = m_currentIndex;
            })
    , m_currentIndex(*property)
{

    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, m_widgetSetter).MakeSafe(m_dispatcherConnections);

    qint32 i(0);
    for(auto* button : buttons) {
        button->setCheckable(true);
        WidgetAbstractButtonWrapper wrapper (button);
        wrapper.OnClicked().Connect(CONNECTION_DEBUG_LOCATION, [i, this]{
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[i, this]{
                m_currentIndex = i;
                m_propertySetter();
                m_widgetSetter();
            });
        }).MakeSafe(m_dispatcherConnections);
        wrapper.WidgetChecked() = false;
        ++i;
    }
    if(*property >= 0 && *property < buttons.size()) {
        WidgetAbstractButtonWrapper(buttons.at(*property)).WidgetChecked() = true;
    }
}

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
    : LocalPropertiesDoubleSpinBoxConnector(property, spinBox,[](double){})
{
    property->Precision.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [spinBox, property]{
        spinBox->setDecimals(property->Precision);
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDoubleSpinBoxConnector::LocalPropertiesDoubleSpinBoxConnector(LocalPropertyDouble* property, QDoubleSpinBox* spinBox, const std::function<void (double)>& propertySetter)
    : Super([spinBox, property](){
                auto precision = epsilon(spinBox->decimals() + 1);
                if(qIsNaN(*property)) {
                    return;
                }
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
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [spinBox, property]{
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
                if(qIsNaN(*property)) {
                    return;
                }
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
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [spinBox, property]{
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
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [spinBox, property]{
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

void LocalPropertiesComboBoxConnector::connectComboBox(QComboBox* comboBox)
{
    m_connections.connect(comboBox, QOverload<qint32>::of(&QComboBox::activated), [this]{
        m_propertySetter();
    });

    if(comboBox->currentIndex() != -1) {
        m_connections.connect(comboBox->model(), &QAbstractItemModel::modelReset, [this]{
            m_widgetSetter();
        });
        m_connections.connect(comboBox->model(), &QAbstractItemModel::rowsInserted,       [this]{ m_widgetSetter(); });
        m_connections.connect(comboBox->model(), &QAbstractItemModel::rowsRemoved,        [this]{ m_widgetSetter(); });
    }
}

void LocalPropertiesComboBoxConnector::setCurrentIndex(QComboBox* comboBox, const std::function<bool (const QModelIndex&)>& handler)
{
    comboBox->setCurrentIndex(ViewModelWrapper(comboBox->model()).IndexOf(handler));
}

QModelIndex LocalPropertiesComboBoxConnector::currentIndex(QComboBox* combobox)
{
    if(combobox->currentIndex() == -1) {
        return QModelIndex();
    }
    return combobox->model()->index(combobox->currentIndex(), 0);
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

    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
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
    property->ConnectBoth(CONNECTION_DEBUG_LOCATION, dateTime->CurrentDate, [](const QDate& dt){ return dt; }, [](const QDate& dt){ return dt; }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [property, dateTime]{
        dateTime->CurrentDate.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

DispatcherConnections connectDateTime(LocalPropertyDateTime* property, WidgetsDateTimeEdit* widgetDateTime, LocalPropertyDoubleOptional* timeShift)
{
    DispatcherConnections result;
    result += property->ConnectBoth(CONNECTION_DEBUG_LOCATION, widgetDateTime->CurrentDateTime, [](const QDateTime& dt){
        return dt;
    },
    [](const QDateTime& dt){
        return dt;
    });
    if(timeShift != nullptr) {
        result += widgetDateTime->TimeShift.ConnectFrom(CONNECTION_DEBUG_LOCATION, *timeShift);
    }
    return result;
}

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDateTime* property, WidgetsDateEdit* dateTime, LocalPropertyDoubleOptional* timeShift)
    : Super([](){},
            [](){}
    )
{
    connectDateTime(property, dateTime, timeShift).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDateTimeConnector::LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, WidgetsDateTimeEdit* dateTime, LocalPropertyDoubleOptional* timeShift)
    : Super([](){},
            [](){}
    )
{
    connectDateTime(property, dateTime, timeShift).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}
