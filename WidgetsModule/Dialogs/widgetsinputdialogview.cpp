#include "widgetsinputdialogview.h"
#include "ui_widgetsinputdialogview.h"

#include <QLineEdit>
#include <QLabel>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/Layouts/widgetsdoublespinboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetslineeditlayout.h"

WidgetsInputDialogView::WidgetsInputDialogView(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsInputDialogView)
{
    ui->setupUi(this);
}

WidgetsInputDialogView::~WidgetsInputDialogView()
{
    delete ui;
}

#ifdef UNITS_MODULE_LIB
void WidgetsInputDialogView::AddMeasurement(const QString& text, const Measurement* measurement, LocalPropertyDouble* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsDoubleSpinBoxLayout();
    widget->label()->setText(text);
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector(measurement, property, widget);
    saveProperty(property);
}
#endif

void WidgetsInputDialogView::AddDouble(const QString& text, LocalPropertyDouble* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsDoubleSpinBoxLayout();
    widget->label()->setText(text);
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector<LocalPropertiesDoubleSpinBoxConnector>(property, widget);
    saveProperty(property);
}

void WidgetsInputDialogView::AddLineText(const QString& text, LocalPropertyString* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsLineEditLayout();
    widget->label()->setText(text);
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(property, widget);
    saveProperty(property);
}

void WidgetsInputDialogView::Reset()
{
    for(const auto& reset : m_resets) {
        reset();
    }
}
