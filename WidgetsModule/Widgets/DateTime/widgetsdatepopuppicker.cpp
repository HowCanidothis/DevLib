#include "widgetsdatepopuppicker.h"
#include "ui_widgetsdatepopuppicker.h"

#include <QWidgetAction>
#include <QMenu>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "widgetsdatetimewidget.h"

WidgetsDatePopupPicker::WidgetsDatePopupPicker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetsDatePopupPicker)
{
    ui->setupUi(this);
    setFocusProxy(ui->DateEdit);
//    ui->CalendarButton->setIcon(IconsManager::GetInstance().GetIcon("CalendarIcon"));

    auto* menu = MenuWrapper(ui->CalendarButton).AddPreventedFromClosingMenu(tr("DateTime"));
    auto* ac = new QWidgetAction(parent);
    m_editor = new WidgetsDateTimeWidget(parent);

    ac->setDefaultWidget(m_editor);

    menu->addAction(ac);
    connect(ui->CalendarButton, &QPushButton::clicked, [menu, this](bool){
        menu->setProperty("a_accept", false);
        m_editor->Store();
        menu->exec(ui->DateEdit->mapToGlobal(ui->DateEdit->geometry().bottomLeft()));
        if(menu->property("a_accept").toBool()) {
            OnDataCommit();
        } else {
            m_editor->Reset();
        }
        OnCloseEditor();
    });
    m_editor->Mode = WidgetsDateTimeWidget::Date;
    m_editor->OnNowActivate.Connect(CONNECTION_DEBUG_LOCATION, [menu]{ menu->setProperty("a_accept", true); menu->close(); });
    m_editor->OnApplyActivate.Connect(CONNECTION_DEBUG_LOCATION, [menu]{ menu->setProperty("a_accept", true); menu->close(); });

    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(&m_editor->CurrentDateTime, ui->DateEdit);
    m_editor->Locale.ConnectFrom(CONNECTION_DEBUG_LOCATION, Locale);
    ui->DateEdit->Locale.ConnectFrom(CONNECTION_DEBUG_LOCATION, Locale);
    ui->DateEdit->DisplayFormat.ConnectFrom(CONNECTION_DEBUG_LOCATION, DisplayFormat);

    WidgetPushButtonWrapper(ui->CalendarButton).SetControl(ButtonRole::DateTimePicker).SetIcon("Calendar");
}

WidgetsDatePopupPicker::~WidgetsDatePopupPicker()
{
    delete ui;
}
