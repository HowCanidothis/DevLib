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
    {
        LocalPropertiesConnectBoth({&Time.OnChange}, [this]{
            if(Time.Native().isValid()) {
                Hours = Time.Native().hour();
                Minutes = Time.Native().minute();
            } else {
                Hours = 0;
                Minutes = 0;
            }
        }, {&Hours.OnChange, &Minutes.OnChange}, [this] {
            Time = QTime(Hours, Minutes);
        });
    }

    LocalPropertyTime Time;
    LocalPropertyInt Hours;
    LocalPropertyInt Minutes;
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

    ui->timePicker->HourType.ConnectFrom(Locale, [](const QLocale& locale){
        return qint32(locale.language() == QLocale::English ? HourFormat::Hour12 : HourFormat::Hour24);
    });

    WidgetsAttachment::Attach(ui->spHours, [this](QObject*, QEvent* e){
        if(e->type() == QEvent::FocusIn) {
            ui->timePicker->TypeClock = ClockType::Hour;
        }
        return false;
    });

    WidgetsAttachment::Attach(ui->spMinutes, [this](QObject*, QEvent* e){
        if(e->type() == QEvent::FocusIn) {
            ui->timePicker->TypeClock = ClockType::Minutes;
        }
        return false;
    });

    ui->timePicker->HourType.SetAndSubscribe([this]{
        auto isInternationFormat = ui->timePicker->HourType.Native() == HourFormat::Hour24;

        ui->btnAM->setVisible(!isInternationFormat);
        ui->btnPM->setVisible(!isInternationFormat);

        if(!isInternationFormat){
            ui->spHours->SetTextFromValueHandler([](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
                value -= value >= 12 ? 12 : 0;
                return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
            });
        } else {
            ui->spHours->SetTextFromValueHandler([](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
                return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
            });
        }
    });
    ui->spMinutes->SetTextFromValueHandler([](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
        return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
    });

    connect(ui->btnAM, &QPushButton::pressed, [this](){ Type = DayType::AM; });
    connect(ui->btnPM, &QPushButton::pressed, [this](){ Type = DayType::PM; });
    Type.SetAndSubscribe([this]{
        ui->btnAM->setProperty("highlighted", Type == DayType::AM);
        ui->btnPM->setProperty("highlighted", Type == DayType::PM);
        StyleUtils::UpdateStyle(ui->btnAM);
        StyleUtils::UpdateStyle(ui->btnPM);
    });

    ui->timePicker->TypeClock.SetAndSubscribe([this]{
        m_connections.clear();
        m_hourTypeConnections.clear();

        switch(ui->timePicker->TypeClock.Native()){
        case ClockType::Hour:
            ui->timePicker->HourType.OnChange.ConnectAndCall(this, [this]{
                m_hourTypeConnections.clear();
                switch (ui->timePicker->HourType.Native()) {
                case HourFormat::Hour12: {
                    LocalPropertiesConnectBoth( {&m_timeConverter->Hours.OnChange}, [this]{
                        const auto& time = m_timeConverter->Hours.Native();
                        if(time <= 12){
                            Type = DayType::AM;
                            ui->timePicker->CurrentTime = time;
                        } else {
                            Type = DayType::PM;
                            ui->timePicker->CurrentTime = time - 12;
                        }
                    }, {&Type.OnChange, &ui->timePicker->CurrentTime.OnChange}, [this]{
                        m_timeConverter->Hours = ui->timePicker->CurrentTime.Native() + (Type == DayType::AM ? 0 : 12);
                    }).MakeSafe(m_hourTypeConnections);
                    break;
                }
                case HourFormat::Hour24:
                    m_timeConverter->Hours.ConnectBoth(ui->timePicker->CurrentTime).MakeSafe(m_hourTypeConnections);
                    break;
                }
            }).MakeSafe(m_connections);
            break;
        case ClockType::Minutes:
            m_timeConverter->Minutes.ConnectBoth(ui->timePicker->CurrentTime).MakeSafe(m_connections);
            break;
        case ClockType::Seconds: Q_ASSERT(false);
//            m_timeConverter->Seconds.ConnectBoth(ui->timePicker->CurrentTime).MakeSafe(m_connections);
            break;
        }
    });
}

WidgetsTimeWidget::~WidgetsTimeWidget()
{
    delete ui;
}
