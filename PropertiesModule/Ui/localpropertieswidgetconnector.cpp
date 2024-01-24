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
#include <QAbstractButton>
#include <QFileDialog>

#include <WidgetsModule/internal.hpp>

#include "styleutils.h"

LocalPropertiesWidgetConnectorBase::LocalPropertiesWidgetConnectorBase(const FAction& widgetSetter, const FAction& propertySetter, QWidget* w)
    : m_widgetSetter([this, widgetSetter](){
        if(!m_ignorePropertyChange) {
            guards::LambdaGuard guard([this]{ m_ignoreWidgetChange = false; }, [this] { m_ignoreWidgetChange = true; } );
            widgetSetter();
        }
    })
    , m_propertySetter([this, propertySetter, widgetSetter]{
        if(!m_ignoreWidgetChange) {
            if(ForceDisabled) {
                guards::LambdaGuard guard2([this]{ m_ignoreWidgetChange = false; }, [this] { m_ignoreWidgetChange = true; } );
                widgetSetter();
                return;
            }
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

    if(w != nullptr) {
        ForceDisabled.Connect(CDL, [w](bool forceDisabled) {
            StyleUtils::ApplyStyleProperty(WidgetProperties::ForceDisabled, w, forceDisabled);
        }).MakeSafe(m_dispatcherConnections);
    }
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
            }, checkBox
    )
{
    property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
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
            }, lineEdit
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

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(Dispatcher* dispatcher, QAbstractButton* button)
    : Super([]{}, [dispatcher]{ dispatcher->Invoke(); }, button)
{
    m_connections.connect(button, &QPushButton::clicked, [this](){
        m_propertySetter();
    });
}

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(LocalPropertyBool* checkedProperty, QAbstractButton* button)
    : Super([button,checkedProperty]{
                button->setChecked(*checkedProperty);
            }, [button,checkedProperty]{
                *checkedProperty = button->isChecked();
            }, button)
{
    Q_ASSERT(button->isCheckable());

    checkedProperty->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(button, &QPushButton::clicked, [this](){
        m_propertySetter();
    });
}

LocalPropertiesLabelConnector::LocalPropertiesLabelConnector(LocalProperty<QImage>* property, QLabel* label, const ImageConnectorParams& params)
    : Super([property, label]{
        auto image = property->Native();
        label->setProperty("a_image", image);
        label->setPixmap(QPixmap::fromImage(image.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }, [property, label]{
        *property = label->property("a_image").value<QImage>();
    }, label)
{
    if(params.BrowseButton != nullptr) {
        WidgetPushButtonWrapper w(params.BrowseButton);
        w.OnClicked().Connect(CDL, [this, property, label, params]{
            QFileDialog fileDialog(WidgetsDialogsManager::GetInstance().GetParentWindow(), "SELECT LOGO");
            if(!params.ForceDefaultDir.isEmpty()) {
                fileDialog.setDirectory(params.ForceDefaultDir);
            }
            fileDialog.setFileMode(QFileDialog::ExistingFile);
            if(fileDialog.exec() != QDialog::Accepted) return;

            auto selectedUrls = fileDialog.selectedUrls();
            auto path = selectedUrls.first().toLocalFile();
            QImage loadedImage;

            if (!loadedImage.load(path)){
                qCWarning(LC_UI) << "Can't load image " << path;
                return;
            }
            label->setProperty("a_image", loadedImage);
            m_propertySetter();
        }).MakeSafe(m_dispatcherConnections);
    }

    SharedPointer<EventFilterObject> ef(WidgetWrapper(label).AddEventFilter([this](QObject*, QEvent* event) -> bool {
        if(event->type() == QEvent::Resize) {
            m_widgetSetter();
        }
        return false;
    }));

    property->OnChanged.Connect(CDL, [this, ef]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    if(params.ClearButton != nullptr) {
        WidgetPushButtonWrapper w(params.ClearButton);
        w.OnClicked().Connect(CDL, [this, label]{
            label->setProperty("a_image", QImage());
            m_propertySetter();
        }).MakeSafe(m_dispatcherConnections);
    }
}

LocalPropertiesLabelConnector::LocalPropertiesLabelConnector(LocalPropertyString* property, ElidedLabel* label)
    : Super([label, property]{
        label->setText(*property);
    }, []{})
{
    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
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
}, textEdit), m_textChanged(250)
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

    ForceDisabled.Connect(CDL, [buttons](bool forceDisabled) {
        for(auto* w : buttons) {
            StyleUtils::ApplyStyleProperty(WidgetProperties::ForceDisabled, w, forceDisabled);
        }
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesPushButtonConnector::LocalPropertiesPushButtonConnector(LocalPropertyColor* property, WidgetsColorPicker* button)
    : Super([button, property]{
        button->Color = property->Native();
    }, [property, button]{
        *property = button->Color.Native();
    }, button)
{
    property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    button->Color.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        m_propertySetter();
    }).MakeSafe(m_dispatcherConnections);
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
    : LocalPropertiesDoubleSpinBoxConnector(property, spinBox, [](double){})
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
            }, spinBox
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
            }, spinBox
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
            }, spinBox
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
    auto old = ForceDisabled.Native();
    if(old) {
        ForceDisabled = false;
    }
    m_connectors.Clear();
    onClear();
    ForceDisabled = old;
}

LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalPropertyName* property, QComboBox* combo)
    : LocalPropertiesComboBoxConnector(property, combo, IdRole)
{}

LocalPropertiesComboBoxConnector::LocalPropertiesComboBoxConnector(LocalPropertyInt* property, QComboBox* combo)
    : LocalPropertiesComboBoxConnector(property, combo, [](const QModelIndex& index) { return index.row(); })
{}

void LocalPropertiesComboBoxConnector::connectComboBox(QComboBox* comboBox)
{
    m_connections.connect(comboBox, QOverload<qint32>::of(&QComboBox::activated), [this]{
        m_propertySetter();
    });

    m_connections.connect(comboBox->model(), &QAbstractItemModel::modelReset, [this]{
        m_widgetSetter();
    });
    m_connections.connect(comboBox->model(), &QAbstractItemModel::rowsInserted,       [this]{ m_widgetSetter(); });
    m_connections.connect(comboBox->model(), &QAbstractItemModel::rowsRemoved,        [this]{ m_widgetSetter(); });
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

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDate* property, WidgetsDateEdit* dateTime)
    : Super([dateTime, property](){
                dateTime->CurrentDate = property->Native();
            },
            [dateTime, property](){
                *property = dateTime->CurrentDate.Native();
            }, dateTime
    )
{
    property->OnChanged.Connect(CDL, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);
    dateTime->CurrentDate.OnChanged.Connect(CDL, [this] {
        m_propertySetter();
    }).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [property, dateTime]{
        dateTime->CurrentDate.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

DispatcherConnections connectDateTime(const FAction& widgetSetter, const FAction& propertySetter, LocalPropertyDateTime* property, WidgetsDateTimeEdit* widgetDateTime, LocalPropertyDoubleOptional* timeShift)
{
    DispatcherConnections result;
    result += property->OnChanged.Connect(CDL, widgetSetter);
    result += widgetDateTime->CurrentDateTime.OnChanged.Connect(CDL, propertySetter);
    if(timeShift != nullptr) {
        result += widgetDateTime->TimeShift.ConnectFrom(CONNECTION_DEBUG_LOCATION, *timeShift);
    }
    return result;
}

LocalPropertiesDateConnector::LocalPropertiesDateConnector(LocalPropertyDateTime* property, WidgetsDateEdit* dateTime, LocalPropertyDoubleOptional* timeShift)
    : Super([dateTime, property](){
                dateTime->CurrentDateTime = property->Native();
            },
            [dateTime, property](){
                *property = dateTime->CurrentDateTime.Native();
            }, dateTime
    )
{
    connectDateTime(m_widgetSetter, m_propertySetter, property, dateTime, timeShift).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}

LocalPropertiesDateTimeConnector::LocalPropertiesDateTimeConnector(LocalPropertyDateTime* property, WidgetsDateTimeEdit* dateTime, LocalPropertyDoubleOptional* timeShift)
    : Super([dateTime, property](){
                dateTime->CurrentDateTime = property->Native();
            },
            [dateTime, property](){
                *property = dateTime->CurrentDateTime.Native();
            }, dateTime
    )
{
    connectDateTime(m_widgetSetter, m_propertySetter, property, dateTime, timeShift).MakeSafe(m_dispatcherConnections);

    property->OnMinMaxChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [property, dateTime]{
        dateTime->CurrentDateTime.SetMinMax(property->GetMin(), property->GetMax());
    }).MakeSafe(m_dispatcherConnections);
}
