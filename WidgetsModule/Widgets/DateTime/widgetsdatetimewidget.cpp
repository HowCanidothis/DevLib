#include "widgetsdatetimewidget.h"
#include "ui_widgetsdatetimewidget.h"

#include <QTextCharFormat>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDateTimeWidget::WidgetsDateTimeWidget(QWidget *parent)
	: Super(parent)
    , NowEnabled(true)
    , ui(new Ui::WidgetsDateTimeWidget)
{
	ui->setupUi(this);
	
	auto style = ui->calendarWidget->weekdayTextFormat(Qt::DayOfWeek::Monday);
	ui->calendarWidget->setWeekdayTextFormat(Qt::DayOfWeek::Saturday, style);
    ui->calendarWidget->setWeekdayTextFormat(Qt::DayOfWeek::Sunday, style);
	
    connect(ui->calendarWidget, &QCalendarWidget::clicked, [this](const QDate& date){
        if(CurrentDateTime.IsRealTime() || !ui->widget->CurrentTime.Native().isValid()) {
            if(TimeShift.IsValid) {
                CurrentDateTime = QDateTime(date, QTime(0,0), Qt::OffsetFromUTC, TimeShift.Value);
                return;
            }
            CurrentDateTime = QDateTime(date, QTime(0,0));
            return;
        }
        if(TimeShift.IsValid) {
            CurrentDateTime = QDateTime(date, CurrentDateTime.Native().time(), Qt::OffsetFromUTC, TimeShift.Value);
            return;
        }
        CurrentDateTime = QDateTime(date, CurrentDateTime.Native().time());
	});

    auto updateTimeRangeHandler = [this]{
        QDateTime dateTime, dateTimeMin, dateTimeMax;
        if(TimeShift.IsValid) {
            dateTime = CurrentDateTime.Native().toOffsetFromUtc(TimeShift.Value);
            dateTimeMin = CurrentDateTime.GetMin().toOffsetFromUtc(TimeShift.Value);
            dateTimeMax = CurrentDateTime.GetMax().toOffsetFromUtc(TimeShift.Value);
        } else {
            dateTime = CurrentDateTime.Native();
            dateTimeMin = CurrentDateTime.GetMin();
            dateTimeMax = CurrentDateTime.GetMax();
        }
        ui->calendarWidget->setSelectedDate(dateTime.date());
        QTime start(0,0), end(23, 59);

        if(!CurrentDateTime.IsRealTime() && dateTimeMin.date() == dateTime.date()){
            start = dateTimeMin.time();
        }
        if(!CurrentDateTime.IsRealTime() && dateTimeMax.date() == dateTime.date()){
            end = dateTimeMax.time();
        }
        Q_ASSERT(!end.isValid() || start <= end);
        ui->calendarWidget->setDateRange(CurrentDateTime.ValidatedMin(dateTimeMin).date(), CurrentDateTime.ValidatedMax(dateTimeMax).date());
        ui->widget->CurrentTime.SetMinMax(start, end);
    };

    TimeShift.Value.Subscribe(updateTimeRangeHandler);
    TimeShift.IsValid.Subscribe(updateTimeRangeHandler);
    CurrentDateTime.SetAndSubscribe(updateTimeRangeHandler);
    CurrentDateTime.OnMinMaxChanged.Connect(CONNECTION_DEBUG_LOCATION, [updateTimeRangeHandler]{
        updateTimeRangeHandler();
	});
	
    CurrentDateTime.ConnectBoth(CONNECTION_DEBUG_LOCATION,ui->widget->CurrentTime, [this](const QDateTime& dt){
        if(TimeShift.IsValid) {
            return dt.toOffsetFromUtc(TimeShift.Value).time();
        }
        return dt.time();
    },
    [this](const QTime& time){
        if(TimeShift.IsValid) {
            return QDateTime(CurrentDateTime.IsRealTime() ? ui->calendarWidget->selectedDate() : CurrentDateTime.Native().date(), time, Qt::OffsetFromUTC, TimeShift.Value);
        }
        return QDateTime(CurrentDateTime.IsRealTime() ? ui->calendarWidget->selectedDate() : CurrentDateTime.Native().date(), time);
    }, TimeShift);
	
	connect(ui->btnNow, &QPushButton::clicked, [this](bool){ 
        CurrentDateTime = QDateTime();
        OnNowActivate();
	});
	connect(ui->btnApply, &QPushButton::clicked, [this](bool){
        OnApplyActivate();
	});

    SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this]{
        ui->calendarWidget->setLocale(SharedSettings::GetInstance().LanguageSettings.ApplicationLocale);
    }).MakeSafe(m_connections);

    WidgetPushButtonWrapper(ui->btnApply).SetControl(ButtonRole::Save);
    WidgetWrapper(ui->widget).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](qint32 mode){
        return mode == DateTime;
    }, Mode);
    WidgetWrapper(ui->btnNow).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, NowEnabled);
}

WidgetsDateTimeWidget::~WidgetsDateTimeWidget()
{
    delete ui;
}

void WidgetsDateTimeWidget::showEvent(QShowEvent* event)
{
    OnAboutToShow();
    Super::showEvent(event);
}

DispatcherConnections WidgetsDateTimeWidget::ConnectModel(const char* locationInfo, LocalPropertyDate* modelProperty)
{
    DispatcherConnections ret;
    auto storedDate = ::make_shared<QDate>();
    ret += Store.Connect(locationInfo, [modelProperty, storedDate]{
        *storedDate = *modelProperty;
    });
    ret += modelProperty->ConnectBoth(locationInfo,CurrentDateTime,
                              [](const QDate& time){ return QDateTime(time, QTime()); },
                              [](const QDateTime& time){ return time.date(); });
    ret += Reset.Connect(locationInfo, [this, storedDate]{
        CurrentDateTime = QDateTime(*storedDate, QTime());
    });
    ret += modelProperty->OnMinMaxChanged.ConnectAndCall(locationInfo, [this, modelProperty]{
        CurrentDateTime.SetMinMax(QDateTime(modelProperty->GetMin(), QTime()), QDateTime(modelProperty->GetMax(), QTime()));
    });

    auto connections = DispatcherConnectionsSafeCreate();
    ret += OnAboutToShow.Connect(locationInfo, [this, modelProperty, connections, locationInfo]{
        connections->clear();
        auto initialDate = modelProperty->Native();
        if(!initialDate.isValid()) initialDate = QDate::currentDate();
        ui->calendarWidget->setCurrentPage(initialDate.year(), initialDate.month());
        WidgetPushButtonWrapper(ui->btnApply).WidgetVisibility().ConnectFrom(locationInfo, [initialDate](const QDate& t){
            return initialDate != t;
        }, *modelProperty).MakeSafe(*connections);
    });
    return ret;
}

DispatcherConnections WidgetsDateTimeWidget::ConnectModel(const char* locationInfo, LocalPropertyDateTime* modelProperty)
{
    DispatcherConnections ret;
    auto storedDate = ::make_shared<QDateTime>();
    ret += Store.Connect(locationInfo, [modelProperty, storedDate]{
        *storedDate = *modelProperty;
    });
    ret += modelProperty->ConnectBoth(locationInfo, CurrentDateTime);
    ret += Reset.Connect(locationInfo, [this, storedDate]{
        CurrentDateTime = *storedDate;
    });
    ret += modelProperty->OnMinMaxChanged.ConnectAndCall(locationInfo, [this, modelProperty]{
        CurrentDateTime.SetMinMax(modelProperty->GetMin(), modelProperty->GetMax());
    });

    auto connections = DispatcherConnectionsSafeCreate();
    ret += OnAboutToShow.Connect(locationInfo, [this, locationInfo, modelProperty, connections]{
        connections->clear();
        auto initialDateTime = modelProperty->Native();
        auto date = initialDateTime.date();
        if(!date.isValid()) date = QDate::currentDate();
        ui->calendarWidget->setCurrentPage(date.year(), date.month());
        WidgetPushButtonWrapper(ui->btnApply).WidgetVisibility().ConnectFrom(locationInfo, [initialDateTime](const QDateTime& dt){
            return initialDateTime != dt;
        }, *modelProperty).MakeSafe(*connections);
    });
    return ret;
}
