#include "widgetsdatetimepopuppicker.h"
#include "ui_widgetsdatetimepopuppicker.h"

#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>
#include <QKeyEvent>

#include <WidgetsModule/internal.hpp>
#include "widgetsdatetimewidget.h"

WidgetsDatetimePopupPicker::WidgetsDatetimePopupPicker(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDatetimePopupPicker)
{
    ui->setupUi(this);
//    ui->CalendarButton->setIcon(IconsManager::GetInstance().GetIcon("CalendarIcon"));
    
    auto* menu = createPreventedFromClosingMenu(tr("DateTime"));
    auto* ac = new QWidgetAction(parent);
    m_editor = new WidgetsDateTimeWidget(parent);
    
    ac->setDefaultWidget(m_editor);
    
    menu->addAction(ac);
    connect(ui->CalendarButton, &QPushButton::clicked, [menu, this](bool){
        menu->exec(ui->dateTimeEdit->mapToGlobal(ui->dateTimeEdit->geometry().bottomLeft()));
        OnCloseEditor();
    });
    m_editor->OnNowActivate.Connect(this, [menu, this]{ OnDataCommit(); menu->close(); });
    m_editor->OnApplyActivate.Connect(this, [menu, this]{ OnDataCommit(); menu->close(); });
    
    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(&m_editor->CurrentDateTime, ui->dateTimeEdit);
    m_editor->Locale.ConnectFrom(Locale);
    ui->dateTimeEdit->Locale.ConnectFrom(Locale);
    ui->dateTimeEdit->DisplayFormat.ConnectFrom(DisplayFormat);
}

WidgetsDatetimePopupPicker::~WidgetsDatetimePopupPicker()
{
    delete ui;
}
