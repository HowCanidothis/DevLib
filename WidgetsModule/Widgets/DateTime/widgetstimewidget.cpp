#include "widgetstimewidget.h"
#include "ui_widgetstimewidget.h"

#include "widgetstimepicker.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

class TimeConverter
{
public:
    TimeConverter()
        : Time(QTime::currentTime())
        , Hours(0, -1, 24)
        , Minutes(0, -1, 60)
    {
        LocalPropertiesConnectBoth(CONNECTION_DEBUG_LOCATION, {&Time.OnChanged}, [this]{
            if(Time.Native().isValid()) {
                Hours = Time.Native().hour();
                Minutes = Time.Native().minute();
            } else {
                Hours = 0;
                Minutes = 0;
            }
        }, {&Hours.OnChanged, &Minutes.OnChanged}, [this] {
            Time = QTime(Hours, Minutes);
        });

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

    if(SharedSettings::IsInitialized()){
        ui->timePicker->HourType.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](const QLocale& locale){
            return qint32(locale.language() == QLocale::English ? HourFormat::Hour12 : HourFormat::Hour24);
        }, SharedSettings::GetInstance().LanguageSettings.ApplicationLocale).MakeSafe(m_connections);
    }

    WidgetWrapper(ui->spHours).AddEventFilter([this](QObject*, QEvent* e){
        if(e->type() == QEvent::FocusIn) {
            ui->timePicker->TypeClock = ClockType::Hour;
        }
        return false;
    });

    WidgetWrapper(ui->spMinutes).AddEventFilter([this](QObject*, QEvent* e){
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

    ui->timePicker->OnMouseReleased.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        if(ui->timePicker->TypeClock == ClockType::Hour) {
            ui->timePicker->TypeClock = ClockType::Minutes;
        }
    });

    ui->timePicker->TypeClock.SetAndSubscribe([this]{
        m_dayTypeConnections.clear();
        m_hourTypeConnections.clear();

        switch(ui->timePicker->TypeClock.Native()){
        case ClockType::Hour:
            ui->timePicker->HourType.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](int hourType){
                m_hourTypeConnections.clear();
                switch (static_cast<HourFormat>(hourType)) {
                case HourFormat::Hour12: {
                    LocalPropertiesConnectBoth(CONNECTION_DEBUG_LOCATION,  {&m_timeConverter->Hours.OnChanged}, [this]{
                        const auto& time = m_timeConverter->Hours.Native();
                        if(time < 12){
                            Type = DayType::AM;
                            ui->timePicker->CurrentTime = time;
                        } else {
                            Type = DayType::PM;
                            ui->timePicker->CurrentTime = time - 12;
                        }
                    }, {&Type.OnChanged, &ui->timePicker->CurrentTime.OnChanged}, [this]{
                        m_timeConverter->Hours = ui->timePicker->CurrentTime.Native() + (Type == DayType::AM ? 0 : 12);
                    }).MakeSafe(m_hourTypeConnections);
                    break;
                }
                case HourFormat::Hour24:
                    m_timeConverter->Hours.ConnectBoth(CONNECTION_DEBUG_LOCATION,ui->timePicker->CurrentTime).MakeSafe(m_hourTypeConnections);
                    break;
                }
            }).MakeSafe(m_dayTypeConnections);
            break;
        case ClockType::Minutes:
            m_timeConverter->Minutes.ConnectBoth(CONNECTION_DEBUG_LOCATION,ui->timePicker->CurrentTime).MakeSafe(m_dayTypeConnections);
            break;
        case ClockType::Seconds: Q_ASSERT(false);
//            m_timeConverter->Seconds.ConnectBoth(CONNECTION_DEBUG_LOCATION, ui->timePicker->CurrentTime).MakeSafe(m_dayTypeConnections);
            break;
        }
    });
}

WidgetsTimeWidget::~WidgetsTimeWidget()
{
    delete ui;
}
