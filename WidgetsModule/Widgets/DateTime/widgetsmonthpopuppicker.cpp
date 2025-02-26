#include "widgetsmonthpopuppicker.h"
#include "ui_widgetsmonthpopuppicker.h"

#include <QMenu>
#include <QWidgetAction>
#include "widgetsmonthpicker.h"

WidgetsMonthPopupPicker::WidgetsMonthPopupPicker(QWidget *parent)
    : QWidget(parent)
    , Enable(true)
    , ShowLeftRight(true)
    , ui(new Ui::WidgetsMonthPopupPicker)
    , m_edit(new WidgetsMonthPicker(this))
{
    ui->setupUi(this);
    ui->dateEdit->Mode = DateTimeDisplayFormatEnum::Month;
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(&Date, ui->dateEdit);

    Enable.ConnectAndCall(CDL, [this](bool enable){
        ui->dateEdit->setReadOnly(!enable);
    });

    WidgetPushButtonWrapper(ui->btnPrevious).SetOnClicked([this]{
        const QDate& date = Date.Native();
        if(date.month() == 1) {
            Date = QDate(date.year() - 1, 12, 1);
        } else {
            Date = QDate(date.year(), date.month() - 1, 1);
        }
    }).WidgetVisibility().ConnectFrom(CDL, [](bool e, bool v){
        return e && v;
    }, Enable, ShowLeftRight);

    WidgetPushButtonWrapper(ui->btnNext).SetOnClicked([this]{
        const QDate& date = Date.Native();
        if(date.month() == 12) {
            Date = QDate(date.year() + 1, 1, 1);
        } else {
            Date = QDate(date.year(), date.month() + 1, 1);
        }
    }).WidgetVisibility().ConnectFrom(CDL, [](bool e, bool v){
        return e && v;
    }, Enable, ShowLeftRight);

    WidgetAbstractButtonWrapper(ui->CalendarButton).SetOnClicked([this]{
        ShowPopup();
        OnCloseEditor();
    }).SetControl(ButtonRole::Icon).SetIcon("Calendar").WidgetVisibility().ConnectFrom(CDL, [](bool e){
        return e;
    }, Enable);

    m_menu = MenuWrapper(parent).AddPreventedFromClosingMenu(tr("MonthPicker"));
    auto* ac = new QWidgetAction(parent);
    ac->setDefaultWidget(m_edit);
    m_menu->addAction(ac);

    m_edit->ConnectModel(CDL, &Date);
    m_edit->OnApply.Connect(CDL, [this]{
        ClosePopup();
    });
    m_edit->OnCancel.Connect(CDL, [this]{
        ClosePopup();
    });
}

WidgetsMonthPopupPicker::~WidgetsMonthPopupPicker()
{
    delete ui;
}

const QDate& WidgetsMonthPopupPicker::GetDate() const {
    return Date.Native();
}

void WidgetsMonthPopupPicker::SetDate(const QDate& d) {
    Date = d;
}

DispatcherConnections WidgetsMonthPopupPicker::ConnectModel(const char* locationInfo, LocalPropertyDate& model) {
    return model.ConnectBoth(CDL, Date);
}

void WidgetsMonthPopupPicker::ShowPopup() {
    m_menu->exec(ui->dateEdit->mapToGlobal(ui->dateEdit->geometry().bottomLeft()));
}

void WidgetsMonthPopupPicker::ClosePopup() {
    m_menu->close();
}
