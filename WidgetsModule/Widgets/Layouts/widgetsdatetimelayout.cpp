#include "widgetsdatetimelayout.h"
#include "ui_widgetsdatetimelayout.h"

#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>
#include <QKeyEvent>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/DateTime/widgetsdatetimeedit.h"
#include "WidgetsModule/Widgets/DateTime/widgetsdatetimewidget.h"

WidgetsDateTimeLayout::WidgetsDateTimeLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDateTimeLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->DateTimePicker->GetLineEdit()).ConnectFocus(ui->label);
    setFocusProxy(ui->DateTimePicker);
}

WidgetsDateTimeLayout::~WidgetsDateTimeLayout()
{
    delete ui;
}

QLabel* WidgetsDateTimeLayout::label() const
{
    return ui->label;
}

WidgetsDatetimePopupPicker* WidgetsDateTimeLayout::dateTime() const
{
    return ui->DateTimePicker;
}

QString WidgetsDateTimeLayout::title() const
{
    return ui->label->text();
}

bool WidgetsDateTimeLayout::isDateTime() const
{
    return ui->DateTimePicker->Mode.Native() == DateTimeDisplayFormatEnum::DateTime;
}

void WidgetsDateTimeLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsDateTimeLayout::setIsDateTime(const bool& dateTime)
{
    ui->DateTimePicker->Mode = dateTime ? DateTimeDisplayFormatEnum::DateTime : DateTimeDisplayFormatEnum::Date;
}

bool WidgetsDateTimeLayout::readOnly() const
{
    return ui->DateTimePicker->GetLineEdit()->isReadOnly();
}

void WidgetsDateTimeLayout::setReadOnly(bool readOnly)
{
    ui->DateTimePicker->GetCalendarButton()->setVisible(!readOnly);
    WidgetWrapper(ui->DateTimePicker->GetLineEdit()).WidgetEnablity() = !readOnly;
}
