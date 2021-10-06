#include "widgetsdatetimewidget.h"
#include "ui_widgetsdatetimewidget.h"

#include <QTextCharFormat>

WidgetsDateTimeWidget::WidgetsDateTimeWidget(QWidget *parent)
	: Super(parent)
    , ui(new Ui::WidgetsDateTimeWidget)
{
	ui->setupUi(this);
	
	auto style = ui->calendarWidget->weekdayTextFormat(Qt::DayOfWeek::Monday);
	ui->calendarWidget->setWeekdayTextFormat(Qt::DayOfWeek::Saturday, style);
    ui->calendarWidget->setWeekdayTextFormat(Qt::DayOfWeek::Sunday, style);
	
    connect(ui->calendarWidget, &QCalendarWidget::clicked, [this](const QDate& date){
        if(CurrentDateTime.IsRealTime() || !ui->widget->CurrentTime.Native().isValid()) {
            CurrentDateTime = QDateTime(date, QTime(0,0));
            return;
        }
        CurrentDateTime = QDateTime(date, CurrentDateTime.Native().time());
	});

    auto updateTimeRangeHandler = [this]{
        ui->calendarWidget->setSelectedDate(CurrentDateTime.Native().date());
        QTime start(0,0), end(23, 59);

        if(!CurrentDateTime.IsRealTime() && CurrentDateTime.GetMin().date() == CurrentDateTime.Native().date()){
            start = CurrentDateTime.GetMin().time();
        }
        if(!CurrentDateTime.IsRealTime() && CurrentDateTime.GetMax().date() == CurrentDateTime.Native().date()){
            end = CurrentDateTime.GetMax().time();
        }
        Q_ASSERT(!end.isValid() || start <= end);
        ui->calendarWidget->setDateRange(CurrentDateTime.GetMin().date(), CurrentDateTime.GetMax().date());
        ui->widget->CurrentTime.SetMinMax(start, end);
    };

    CurrentDateTime.SetAndSubscribe(updateTimeRangeHandler);
    CurrentDateTime.OnMinMaxChanged.Connect(this, [updateTimeRangeHandler]{
        updateTimeRangeHandler();
	});
	
    CurrentDateTime.ConnectBoth(ui->widget->CurrentTime, [](const QDateTime& dt){ return dt.time(); },
    [this](const QTime& time){
        return QDateTime(CurrentDateTime.IsRealTime() ? ui->calendarWidget->selectedDate() : CurrentDateTime.Native().date(), time);
    });
	
	connect(ui->btnNow, &QPushButton::clicked, [this](bool){ 
        CurrentDateTime = QDateTime();
        OnNowActivate();
	});
	connect(ui->btnApply, &QPushButton::clicked, [this](bool){
        OnApplyActivate();
	});

    Locale.OnChange += { this, [this]{ ui->calendarWidget->setLocale(Locale); } };
}

WidgetsDateTimeWidget::~WidgetsDateTimeWidget()
{
    delete ui;
}

void WidgetsDateTimeWidget::ConnectModel(LocalPropertyDateTime* modelProperty, bool reactive)
{
    if(reactive) {
        modelProperty->ConnectBoth(CurrentDateTime,
                                  [](const QDateTime& time){ return time; },
                                  [](const QDateTime& time){ return time; }).MakeSafe(m_connections);
    } else {
        CurrentDateTime = modelProperty->Native();
        OnApplyActivate.Connect(this, [modelProperty, this]{
            *modelProperty = CurrentDateTime.Native();
        });
    }
    modelProperty->OnMinMaxChanged.Connect(this, [this, modelProperty]{
        CurrentDateTime.SetMinMax(modelProperty->GetMin(), modelProperty->GetMax());
    }).MakeSafe(m_connections);
    CurrentDateTime.SetMinMax(modelProperty->GetMin(), modelProperty->GetMax());
}
