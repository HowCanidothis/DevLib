#include "widgetsinputdialogview.h"
#include "ui_widgetsinputdialogview.h"

#include <QLineEdit>
#include <QLabel>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/Layouts/widgetsdoublespinboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetslineeditlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetsdatetimelayout.h"
#include "WidgetsModule/Widgets/DateTime/widgetsmonthpicker.h"

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

void WidgetsInputDialogView::AddInt(const QString& label, LocalPropertyInt* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsSpinBoxLayout();
    if(count == 1) {
        setFocusProxy(widget);
    }
    widget->label()->setText(label);
    ui->Layout->addWidget(widget, count, 0);
    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(property, widget);
    saveProperty(property);
}

void WidgetsInputDialogView::AddLineText(const QString& text, LocalPropertyString* property, const QStringList& keys, bool encrypted)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsLineEditLayout();
    if(!keys.isEmpty()) {
        WidgetLineEditWrapper(widget->lineEdit()).AddCompleter(keys);
    }
    if(encrypted) {
        WidgetLineEditWrapper(widget->lineEdit()).AddPasswordButton();
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

void WidgetsInputDialogView::AddMonth(const QString& label, LocalPropertyDate* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsMonthPicker();
    widget->layout()->setContentsMargins(0,0,0,0);
    widget->EnableButtons = false;
    if(count == 1) {
        setFocusProxy(widget);
    }
//    widget->label()->setText(label);
    ui->Layout->addWidget(widget, count, 0);
    property->ConnectBoth(CDL, widget->Date).MakeSafe(m_connections);
    saveProperty(property);
}

void WidgetsInputDialogView::AddDateTime(const QString& label, LocalPropertyDateTime* property)
{
    auto count = ui->Layout->rowCount();
    auto* widget = new WidgetsDateTimeLayout();
    if(count == 1) {
        setFocusProxy(widget);
    }
    widget->label()->setText(label);
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
    toDtWidget->setIsDateTime(false);
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
