#include "widgetsdatetimepopuppicker.h"
#include "ui_widgetsdatetimepopuppicker.h"

#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>
#include <QKeyEvent>

#include "widgetsdatetimewidget.h"

WidgetsDatetimePopupPicker::WidgetsDatetimePopupPicker(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDatetimePopupPicker)
{
    ui->setupUi(this);
    auto updateLocale = [this]{
        const auto& locale = Locale.Native();
        if(locale == QLocale::English){
            DisplayFormat = "MM/dd/yy hh:mm AP";
        } else {
            DisplayFormat = locale.dateTimeFormat(QLocale::FormatType::ShortFormat);
        }
    };
    updateLocale();
    Locale.OnChange.Connect(this, updateLocale).MakeSafe(m_connections);
    
    auto* menu = createPreventedFromClosingMenu(tr("DateTime"));
    auto* ac = new QWidgetAction(parent);
    m_editor = new WidgetsDateTimeWidget(parent);
    m_editor->CurrentDateTime.ConnectBoth(ui->dateTimeEdit->IsValid, [](const QDateTime& dt) {
        return dt.isValid();
    }, [this](bool isValid){
        return isValid ? m_editor->CurrentDateTime.Native() : QDateTime();
    });
    
    ac->setDefaultWidget(m_editor);
    
    menu->addAction(ac);
    connect(ui->CalendarButton, &QPushButton::clicked, [menu, this](bool){
        menu->exec(ui->dateTimeEdit->mapToGlobal(ui->dateTimeEdit->geometry().bottomLeft()));
        OnCloseEditor();
    });
    m_editor->OnNowActivate.Connect(this, [menu, this]{ OnDataCommit(); menu->close(); });
    m_editor->OnApplyActivate.Connect(this, [menu, this]{ OnDataCommit(); menu->close(); });
    
    m_editor->CurrentDateTime.OnMinMaxChanged.Connect(this, [this]{
        ui->dateTimeEdit->setDateTimeRange(m_editor->CurrentDateTime.GetMinValid(), m_editor->CurrentDateTime.GetMaxValid());
    });
    m_connectors.AddConnector<LocalPropertiesWidgetsDateTimeConnector>(&m_editor->CurrentDateTime, ui->dateTimeEdit);
    m_editor->Locale.ConnectFrom(Locale);
    ui->dateTimeEdit->Locale.ConnectFrom(Locale);
    ui->dateTimeEdit->DisplayFormat.ConnectFrom(DisplayFormat);
}

WidgetsDatetimePopupPicker::~WidgetsDatetimePopupPicker()
{
    delete ui;
}
