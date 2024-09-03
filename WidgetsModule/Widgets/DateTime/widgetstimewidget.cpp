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

    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(&m_hoursUI, ui->spHours);
    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(&m_timeConverter->Minutes, ui->spMinutes);


    if(SharedSettings::IsInitialized()){
        ui->timePicker->HourType.ConnectFrom(CDL, [](const QLocale& locale){
            return qint32(locale.language() == QLocale::English ? HourFormat::Hour12 : HourFormat::Hour24);
        }, SharedSettings::GetInstance().LanguageSettings.ApplicationLocale).MakeSafe(m_connections);
    }

    auto formatConnections = DispatcherConnectionsSafeCreate();
    ui->timePicker->HourType.ConnectAndCall(CDL, [this, formatConnections](int type){
        formatConnections->clear();
        auto isInternationFormat = type == (int)HourFormat::Hour24;

        ui->btnAM->setVisible(!isInternationFormat);
        ui->btnPM->setVisible(!isInternationFormat);

        if(isInternationFormat) {
            m_hoursUI.SetValidator([](qint32 value){
                if(value >= 24) {
                    return 0;
                } else if(value < 0) {
                    return 23;
                }
                return value;
            });
            m_timeConverter->Hours.ConnectBoth(CDL, m_hoursUI).MakeSafe(*formatConnections);
        } else {
            m_hoursUI.SetValidator([this](qint32 value){
                if(value >= 12) {
                    Type = Type.Native() == DayType::AM ? DayType::PM : DayType::AM;
                    return 0;
                } else if(value < 0) {
                    Type = Type.Native() == DayType::AM ? DayType::PM : DayType::AM;
                    return 11;
                }
                return value;
            });
            LocalPropertiesConnectBoth(CDL,  {&m_timeConverter->Hours.OnChanged}, [this]{
                const auto& time = m_timeConverter->Hours.Native();
                if(time < 12){
                    Type = DayType::AM;
                    m_hoursUI = time;
                } else {
                    Type = DayType::PM;
                    m_hoursUI = time - 12;
                }
            }, {&m_hoursUI.OnChanged, &Type.OnChanged}, [this]{
                m_timeConverter->Hours = m_hoursUI.Native() + (Type == DayType::AM ? 0 : 12);
            }).MakeSafe(*formatConnections);
        }
    });

    auto hourMinutesConnections = DispatcherConnectionsSafeCreate();
    ui->timePicker->TypeClock.ConnectAndCall(CDL, [this, hourMinutesConnections](int typeClock){
        hourMinutesConnections->clear();
        switch(static_cast<ClockType>(typeClock)){
        case ClockType::Hour:
            m_hoursUI.ConnectBoth(CDL, ui->timePicker->CurrentTime).MakeSafe(*hourMinutesConnections); break;
        case ClockType::Minutes:
            m_timeConverter->Minutes.ConnectBoth(CDL, ui->timePicker->CurrentTime).MakeSafe(*hourMinutesConnections); break;
        default: Q_ASSERT(false); break;
        }
    });

    static auto DisplayFormat([](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
        return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
    });

    ui->spHours->SetTextFromValueHandler(DisplayFormat);
    ui->spMinutes->SetTextFromValueHandler(DisplayFormat);

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
    ui->timePicker->OnMouseReleased.Connect(CDL, [this]{
        if(ui->timePicker->TypeClock == ClockType::Hour) {
            ui->timePicker->TypeClock = ClockType::Minutes;
        }
    });

    WidgetPushButtonWrapper(ui->btnAM).SetOnClicked([this]{ Type = DayType::AM; });
    WidgetPushButtonWrapper(ui->btnPM).SetOnClicked([this]{ Type = DayType::PM; });
    Type.SetAndSubscribe([this]{
        ui->btnAM->setProperty("highlighted", Type == DayType::AM);
        ui->btnPM->setProperty("highlighted", Type == DayType::PM);
        WidgetWrapper(ui->btnAM).UpdateStyle();
        WidgetWrapper(ui->btnPM).UpdateStyle();
    });
}

WidgetsTimeWidget::~WidgetsTimeWidget()
{
    delete ui;
}
