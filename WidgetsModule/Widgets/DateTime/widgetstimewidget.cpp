#include "widgetstimewidget.h"
#include "ui_widgetstimewidget.h"

#include "widgetstimepicker.h"
#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

class TimeConverter
{
public:
    TimeConverter()
        : Time(QTime::currentTime())
        , Hours(0, -1, 24)
        , Minutes(0, -1, 60)
        , m_recursionLock(false)
    {
        auto splitTime = [this]{
            if(m_recursionLock){
                return;
            }
            guards::BooleanGuard guard(&m_recursionLock);
            if(Time.Native().isValid()) {
                Hours = Time.Native().hour();
                Minutes = Time.Native().minute();
            } else {
                Hours = 0;
                Minutes = 0;
            }
        };

        auto buildTime = [this]{
            if(m_recursionLock){
                return;
            }
            guards::BooleanGuard guard(&m_recursionLock);
            Time = QTime(Hours, Minutes);
        };

        Hours.SetValidator([](qint32 value){
            if(value >= 24) {
                return 0;
            } else if(value < 0) {
                return 23;
            }
            return value;
        });

        Minutes.SetValidator([this](qint32 value){
            if(value >= 60) {
                Hours += 1;
                return 0;
            } else if(value < 0) {
                Hours -= 1;
                return 59;
            }
            return value;
        });

        Time.OnChange += { this, splitTime};
        Hours.OnChange += { this, buildTime};
        Minutes.OnChange += { this, buildTime};

        splitTime();
    }

    LocalPropertyTime Time;
    LocalPropertyInt Hours;
    LocalPropertyInt Minutes;

private:
    bool m_recursionLock;
};

WidgetsTimeWidget::WidgetsTimeWidget(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsTimeWidget)
    , m_timeConverter(new TimeConverter())
    , CurrentTime(m_timeConverter->Time)
{
    ui->setupUi(this);
    ui->label->setProperty("splitter", true);

    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(&m_timeConverter->Hours, ui->spHours);
    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(&m_timeConverter->Minutes, ui->spMinutes);
    connect(ui->btnAM, &QPushButton::pressed, [this](){ Type = DayType::AM; });
    connect(ui->btnPM, &QPushButton::pressed, [this](){ Type = DayType::PM; });

    Locale.Subscribe([this]{ ui->timePicker->HourType = Locale.Native() == QLocale::English ? HourFormat::Hour12 : HourFormat::Hour24; });
    auto connections = ::make_shared<DispatcherConnectionsSafe>();
    ui->timePicker->HourType.SetAndSubscribe([this, connections]{
        connections->clear();

        auto visible = ui->timePicker->HourType.Native() == HourFormat::Hour12;
        ui->btnAM->setVisible(visible);
        ui->btnPM->setVisible(visible);

        if(visible){
            m_timeConverter->Hours.ConnectBoth(Type, [](qint32 value) -> qint32 {
                return qint32(value > 12 ? DayType::PM : DayType::AM);
            }, [this](qint32 value) -> qint32 {
                return m_timeConverter->Hours + ((value == (qint32)DayType::AM) ? -12 : 12);
            }).MakeSafe(*connections);
        }
    });

    auto updateButtonState = [this]{
        ui->btnAM->setProperty("highlighted", Type == DayType::AM);
        ui->btnPM->setProperty("highlighted", Type == DayType::PM);
        StyleUtils::UpdateStyle(ui->btnAM);
        StyleUtils::UpdateStyle(ui->btnPM);
    };
   
    m_timeConverter->Hours.ConnectBoth(Type, [](qint32 value) -> qint32 {
        return qint32(value >= 12 ? DayType::PM : DayType::AM);
    }, [this](qint32 value) -> qint32 {
        return m_timeConverter->Hours + ((value == (qint32)DayType::AM) ? -12 : 12);
    });

    Type.SetAndSubscribe(updateButtonState);
    
    auto connectHours = [this]{
        m_connections.clear();
        
        ui->timePicker->TypeClock = ClockType::Hour;
        ui->timePicker->HourType.OnChange.ConnectAndCall(this, [this]{
            m_hourTypeConnections.clear();
            switch (ui->timePicker->HourType.Native()) {
            case HourFormat::Hour12:
                m_timeConverter->Hours.ConnectBoth(ui->timePicker->CurrentTime, [this](qint32 value){
                    return value + ((Type == DayType::AM) ? 0 : -12);
                }, [this](qint32 value){
                    return value + ((Type == DayType::AM) ? 0 : 12);
                }).MakeSafe(m_hourTypeConnections);
                break;
            case HourFormat::Hour24:
                m_timeConverter->Hours.ConnectBoth(ui->timePicker->CurrentTime).MakeSafe(m_hourTypeConnections);
                break;
            }
        }).MakeSafe(m_connections);
    };
    WidgetsAttachment::Attach(ui->spHours, [connectHours](QObject*, QEvent* e){
        if(e->type() == QEvent::FocusIn) {
            connectHours();
        }
        return false;
    });
    connectHours();

    WidgetsAttachment::Attach(ui->spMinutes, [this](QObject*, QEvent* e){
        if(e->type() == QEvent::FocusIn) {
            m_connections.clear();
            
            m_hourTypeConnections.clear();
            ui->timePicker->TypeClock = ClockType::Minutes;
            m_timeConverter->Minutes.ConnectBoth(ui->timePicker->CurrentTime).MakeSafe(m_connections);
        }
        return false;
    });

    
    auto addZeroHoursHandler = [this](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
        value = ui->timePicker->HourType.Native() == HourFormat::Hour12 && value >= 12 ? (value - 12) : value;
        return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
    };

    auto addZeroHandler = [](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
        return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
    };
    ui->spHours->SetTextFromValueHandler(addZeroHoursHandler);
    ui->spMinutes->SetTextFromValueHandler(addZeroHandler);
}

WidgetsTimeWidget::~WidgetsTimeWidget()
{
    delete ui;
}
