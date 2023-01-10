#include "widgetsinputdialog.h"
#include "ui_widgetsinputdialog.h"

#include <QLineEdit>
#include <QLabel>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/widgetsspinboxwithcustomdisplay.h"

WidgetsInputDialog::WidgetsInputDialog(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsInputDialog)
{
    ui->setupUi(this);
    setWindowTitle(QString());

    WidgetPushButtonWrapper(ui->BtnOk).SetControl(ButtonRole::Save).SetOnClicked([this]{ accept(); });
    WidgetPushButtonWrapper(ui->BtnCancel).SetControl(ButtonRole::Cancel).SetOnClicked([this]{
        for(const auto& reset : m_resets) {
            reset();
        }
        reject();
    });
}

WidgetsInputDialog::~WidgetsInputDialog()
{
    delete ui;
}

#ifdef UNITS_MODULE_LIB
void WidgetsInputDialog::AddMeasurement(const QString& text, const Measurement* measurement, LocalPropertyDouble* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsDoubleSpinBoxWithCustomDisplay();
    auto* label = new QLabel(text);
    auto* unit = new QLabel();
    ui->Layout->addWidget(label, count, 0);
    ui->Layout->addWidget(widget, count, 1);
    ui->Layout->addWidget(unit, count, 2);
    m_connectors.AddConnector(measurement, property, widget, unit);
    saveProperty(property);
}
#endif

void WidgetsInputDialog::AddDouble(const QString& text, LocalPropertyDouble* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsDoubleSpinBoxWithCustomDisplay();
    auto* label = new QLabel(text);
    ui->Layout->addWidget(label, count, 0);
    ui->Layout->addWidget(widget, count, 1);
    m_connectors.AddConnector<LocalPropertiesDoubleSpinBoxConnector>(property, widget);
    saveProperty(property);
}

void WidgetsInputDialog::AddLineText(const QString& text, LocalPropertyString* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new QLineEdit();
    auto* label = new QLabel(text);
    ui->Layout->addWidget(label, count, 0);
    ui->Layout->addWidget(widget, count, 1);
    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(property, widget);
    saveProperty(property);
}
