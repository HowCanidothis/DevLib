#include "widgetstimewidget.h"
#include "ui_widgetstimewidget.h"

#include "widgetstimepicker.h"
#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsTimeWidget::WidgetsTimeWidget(QWidget *parent)
    : Super(parent)
	, CurrentTime(QTime(0,0))
    , Hour(0, 0, 12)
    , Minutes(0, 0, 59)
    , ui(new Ui::WidgetsTimeWidget)
{
    ui->setupUi(this);
    ui->label->setProperty("splitter", true);
    
    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(&Hour, ui->spHours);
    m_connectors.AddConnector<LocalPropertiesSpinBoxConnector>(&Minutes, ui->spMinutes);
    connect(ui->btnAM, &QPushButton::pressed, [this](){ Type = DayType::AM; });
    connect(ui->btnPM, &QPushButton::pressed, [this](){ Type = DayType::PM; });
    
    auto complexTimeLocker = ::make_shared<bool>(false);
    CurrentTime.SetAndSubscribe([this, complexTimeLocker]{
        if(*complexTimeLocker){
            return;
        }
        guards::LambdaGuard([complexTimeLocker]{ *complexTimeLocker = false; }, [complexTimeLocker]{ *complexTimeLocker = true; });
        Type = CurrentTime.Native().hour() < 12 ?  DayType::AM :  DayType::PM;
        Hour = CurrentTime.Native().hour() - (Type == DayType::AM ? 0 : 12);
        Minutes = CurrentTime.Native().minute();
    });
    auto buildTime = [this, complexTimeLocker]{
        if(*complexTimeLocker){
            return;
        }
        guards::LambdaGuard([complexTimeLocker]{ *complexTimeLocker = false; }, [complexTimeLocker]{ *complexTimeLocker = true; });
        CurrentTime = QTime(Hour + (Type == DayType::AM ? 0 : 12), Minutes);
    };
    
    auto updateDelayMinutes = ::make_shared<DelayedCallObject>();
    auto updateMinutesRange = [this, updateDelayMinutes]{
        updateDelayMinutes->Call([this, updateDelayMinutes]{
            int startMinutes = 0, stopMinutes = 59;
            if(Hour.GetMin() == Hour.Native()){
                startMinutes = CurrentTime.GetMin().minute();
            }
            if(Hour.GetMax() == Hour.Native()){
                const auto& max = CurrentTime.GetMax();
                stopMinutes = max.isValid() ? CurrentTime.GetMax().minute() : QTime::currentTime().minute();
            }
            Minutes.SetMinMax(startMinutes, stopMinutes);
        });
    };
    
    auto updateDelayHours = ::make_shared<DelayedCallObject>();
    auto updateHourRange = [this, updateMinutesRange, updateDelayHours]{
        updateDelayHours->Call([this, updateMinutesRange]{
            const auto& min = CurrentTime.GetMin().hour();
            const auto& max = CurrentTime.GetMax().isValid() ? CurrentTime.GetMax().hour() : 24;
            
            switch (Type.Native()) {
            case DayType::AM: {
                Hour.SetMinMax(min, qMin(max, 12));
                break;
            }
            case DayType::PM: {
                Hour.SetMinMax(qMax(0, min - 12), max - 12);
                break;
            }}
            updateMinutesRange();
        });
    };
    
    auto updateRange = [this,updateHourRange]{
        ui->btnAM->setEnabled(CurrentTime.GetMin().hour() < 12);
        ui->btnPM->setEnabled((CurrentTime.GetMax().isValid() ? CurrentTime.GetMax().hour() : QTime::currentTime().hour()) >= 12);
        if(Type == DayType::AM && !ui->btnAM->isEnabled()){
            Type = DayType::PM;
        } else if(Type == DayType::PM && !ui->btnPM->isEnabled()){
            Type = DayType::AM;
        } else {
            updateHourRange();
        }
    };
    CurrentTime.OnMinMaxChanged.Connect(this, updateRange);
    
    auto updateButtonState = [this]{
        ui->btnAM->setProperty("highlighted", Type == DayType::AM);
        ui->btnPM->setProperty("highlighted", Type == DayType::PM);
        StyleUtils::UpdateStyle(ui->btnAM);
        StyleUtils::UpdateStyle(ui->btnPM);
    };
    updateButtonState();
    Type.Subscribe([=]{
        buildTime();
        updateHourRange();
        updateButtonState();
    });

    Hour.Subscribe([buildTime, updateMinutesRange]{
        buildTime();
        updateMinutesRange();
    });
    Minutes.Subscribe(buildTime);
    
    auto connectHours = [this]{
        m_connections.clear();
        ui->timePicker->Type = ClockType::Hour;

        auto updateRange = [this]{ ui->timePicker->CurrentTime.SetMinMax(Hour.GetMin(), Hour.GetMax()); };
        updateRange();
        Hour.OnMinMaxChanged.Connect(this, updateRange).MakeSafe(m_connections);
        Hour.ConnectBoth(ui->timePicker->CurrentTime, [](int h){ return h; }, [](int h){ return h; }).MakeSafe(m_connections);
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
            ui->timePicker->Type = ClockType::Minutes;
            auto updateRange = [this]{ ui->timePicker->CurrentTime.SetMinMax(Minutes.GetMin(), Minutes.GetMax()); };
            updateRange();
            Minutes.OnMinMaxChanged.Connect(this, updateRange).MakeSafe(m_connections);
            Minutes.ConnectBoth(ui->timePicker->CurrentTime, [](int m){return m; }, [](int m){return m; }).MakeSafe(m_connections);
        }
        return false;
    });

    auto addZeroHandler = [](const WidgetsSpinBoxWithCustomDisplay*, qint32 value)->QString {
        return QString("%1%2").arg(abs(value) < 10 ? "0" : "").arg(value);
    };
    ui->spHours->SetTextFromValueHandler(addZeroHandler);
    ui->spMinutes->SetTextFromValueHandler(addZeroHandler);
}

WidgetsTimeWidget::~WidgetsTimeWidget()
{
    delete ui;
}
