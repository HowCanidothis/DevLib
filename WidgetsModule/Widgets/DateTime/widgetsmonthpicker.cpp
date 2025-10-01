#include "widgetsmonthpicker.h"
#include "ui_widgetsmonthpicker.h"

WidgetsMonthPicker::WidgetsMonthPicker(QWidget *parent)
    : QFrame(parent)
    , EnableButtons(true)
    , ui(new Ui::WidgetsMonthPicker)
{
    ui->setupUi(this);
    m_currButtons = {ui->btnJan, ui->btnFeb, ui->btnMar, ui->btnApr,
                     ui->btnMay, ui->btnJun, ui->btnJul, ui->btnAug,
                     ui->btnSep, ui->btnOct, ui->btnNov, ui->btnDec};

    WidgetPushButtonWrapper(ui->btnPrevious).SetOnClicked([this]{
        Date = Date.Native().addMonths(-1);
    }).WidgetVisibility().ConnectFrom(CDL, [this](const QDate& d){
        auto min = Date.GetMin();
        if(!min.isValid()){
            return true;
        }
        return  min.year() < d.year() || min.month() < d.month();
    }, Date);

    WidgetPushButtonWrapper(ui->btnNext).SetOnClicked([this]{
        Date = Date.Native().addMonths(1);
    }).WidgetVisibility().ConnectFrom(CDL, [this](const QDate& d){
        auto max = Date.GetMax();
        if(!max.isValid()){
            return true;
        }
        return  d.year() < max.year() || d.month() < max.month();
    }, Date);

    int month(1);
    for(auto* btn : m_currButtons){
        WidgetPushButtonWrapper(btn).SetText([month]{
            const auto& locale = SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.Native();
            return locale.monthName(month, QLocale::ShortFormat);
        }, SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.OnChanged).WidgetEnablity().ConnectFrom(CDL, [this, month](const QDate& year){
            auto date = QDate(year.year(), month, 1);
            const auto& min = Date.GetMin();
            const auto& max = Date.GetMax();
            if(min.isValid() && date < min) {
                return false;
            }
            if(max.isValid() && max < date){
                return false;
            }
            return true;
        }, Date);
        btn->setProperty("month", month);
        ++month;
    }

    Date.ConnectBoth(CDL, m_buttonIndex, [](const QDate& date){
        return date.month() - 1;
    }, [this](int index){
        return QDate(Date.Native().year(), index + 1, 1);
    });

    ui->dateEdit->Mode = DateTimeDisplayFormatEnum::Month;
    for(auto button : m_currButtons)WidgetAbstractButtonWrapper(button).SetControl(ButtonRole::Tab);
    m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(&m_buttonIndex, m_currButtons);
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(&Date, ui->dateEdit);

    WidgetPushButtonWrapper(ui->btnSave).SetControl(ButtonRole::Save).CreateVisibilityRule(CDL, [this]{
        return EnableButtons.Native();
    }, {ui->btnCancel}, EnableButtons);

    WidgetPushButtonWrapper(ui->btnSave).OnClicked().Connect(CDL, [this]{
        OnApply();
    });
    WidgetPushButtonWrapper(ui->btnCancel).OnClicked().Connect(CDL, [this]{
        OnCancel();
    });
}

WidgetsMonthPicker::~WidgetsMonthPicker()
{
    delete ui;
}

DispatcherConnections WidgetsMonthPicker::ConnectModel(const char* locationInfo, LocalPropertyDate* model){
    DispatcherConnections result;
    result += Date.ConnectFrom(locationInfo, *model);
    result += model->OnMinMaxChanged.ConnectAndCall(locationInfo, [this, model]{
        Date.SetMinMax(model->GetMin(), model->GetMax());
    });
    result += OnApply.Connect(locationInfo, [this, model]{
        *model = Date.Native();
    });
    return result;
}

DispatcherConnections WidgetsMonthPicker::ConnectModel(const char* locationInfo, LocalPropertyInt* month, LocalPropertyInt* year){
    DispatcherConnections result;
    result += Date.ConnectFrom(locationInfo, [](int m, int y){
        return QDate(y, m, 1);
    }, *month, *year);
    result += month->OnMinMaxChanged.ConnectAndCallCombined(locationInfo, [this, month, year]{
        Date.SetMinMax(QDate(year->GetMin(), month->GetMin(), 1), QDate(year->GetMax(), month->GetMax(), 1));
    }, year->OnMinMaxChanged);
    result += OnApply.Connect(locationInfo, [this, month, year]{
        *month = Date.Native().month();
        *year = Date.Native().year();
    });
    return result;
}
