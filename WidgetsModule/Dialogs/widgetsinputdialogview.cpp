#include "widgetsinputdialogview.h"
#include "ui_widgetsinputdialogview.h"

#include <QLineEdit>
#include <QLabel>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/Layouts/widgetsdoublespinboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetslineeditlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetsdatetimelayout.h"

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
    if(count == 1) {
        setFocusProxy(widget);
    }
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
    if(count == 1) {
        setFocusProxy(widget);
    }
    widget->label()->setText(text);
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector<LocalPropertiesDoubleSpinBoxConnector>(property, widget);
    saveProperty(property);
}

void WidgetsInputDialogView::AddLineText(const QString& text, LocalPropertyString* property, const QStringList& keys)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsLineEditLayout();
    if(!keys.isEmpty()) {
        WidgetLineEditWrapper(widget->lineEdit()).AddCompleter(keys);
    }
    widget->label()->setText(text);
    if(count == 1) {
        setFocusProxy(widget);
    }
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(property, widget, false);
    saveProperty(property);
}

void WidgetsInputDialogView::AddDate(const QString& text, LocalPropertyDate* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsDateTimeLayout();
    if(count == 1) {
        setFocusProxy(widget);
    }
    widget->setIsDateTime(false);
    widget->label()->setText(text);
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(property, widget);
    saveProperty(property);
}

void WidgetsInputDialogView::AddDateRange(const QString& label, LocalPropertyDate* from, LocalPropertyDate* to)
{
    auto count = ui->Layout->rowCount();
    auto* horizontal = new QHBoxLayout();
    auto* fromDtWidget = new WidgetsDateTimeLayout();
    auto* toDtWidget = new WidgetsDateTimeLayout();
    if(count == 1) {
        setFocusProxy(fromDtWidget);
    }
    fromDtWidget->setIsDateTime(false);
    fromDtWidget->label()->setText(tr("From"));
    toDtWidget->label()->setText(tr("To"));
    horizontal->addWidget(fromDtWidget);
    horizontal->addWidget(toDtWidget);
    ui->Layout->addLayout(horizontal, count, 0);
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(from, fromDtWidget);
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(to, toDtWidget);
    saveProperty(from);
    saveProperty(to);
}

void WidgetsInputDialogView::AddDateTimeRange(const QString& label, LocalPropertyDateTime* from, LocalPropertyDateTime* to)
{
    auto count = ui->Layout->rowCount();
    auto* horizontal = new QHBoxLayout();
    auto* fromDtWidget = new WidgetsDateTimeLayout();
    auto* toDtWidget = new WidgetsDateTimeLayout();
    if(count == 1) {
        setFocusProxy(fromDtWidget);
    }
    fromDtWidget->setIsDateTime(true);
    fromDtWidget->label()->setText(tr("From"));
    toDtWidget->label()->setText(tr("To"));
    horizontal->addWidget(fromDtWidget);
    horizontal->addWidget(toDtWidget);
    ui->Layout->addLayout(horizontal, count, 0);
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(from, fromDtWidget);
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(to, toDtWidget);
    saveProperty(from);
    saveProperty(to);
}

void WidgetsInputDialogView::Reset()
{
    for(const auto& reset : m_resets) {
        reset();
    }
}
