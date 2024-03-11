#include "widgetsdatetimelayout.h"
#include "ui_widgetsdatetimelayout.h"

#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>
#include <QKeyEvent>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/DateTime/widgetsdatetimeedit.h"
#include "WidgetsModule/Widgets/DateTime/widgetsdatetimewidget.h"

WidgetsDateTimeLayout::WidgetsDateTimeLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDateTimeLayout)
{
    ui->setupUi(this);

    setFocusProxy(ui->dateTimeEdit);
    WidgetPushButtonWrapper(ui->CalendarButton).WidgetVisibility().ConnectFrom(CDL, FInverseBool, ForceDisabled);
    m_connectors.ForceDisabled.ConnectFrom(CDL, ForceDisabled);

    auto* menu = MenuWrapper(ui->CalendarButton).AddPreventedFromClosingMenu(tr("DateTime"));
    auto* ac = new QWidgetAction(parent);
    m_editor = new WidgetsDateTimeWidget(parent);
    m_editor->TimeShift.ConnectFrom(CDL, TimeShift);

    ac->setDefaultWidget(m_editor);

    menu->addAction(ac);
    connect(ui->CalendarButton, &QPushButton::clicked, [menu, this](bool){
        menu->setProperty("a_accept", false);
        m_editor->Store();
        menu->exec(ui->dateTimeEdit->mapToGlobal(ui->dateTimeEdit->geometry().bottomLeft()));
        if(menu->property("a_accept").toBool()) {
            OnDataCommit();
        } else {
            m_editor->Reset();
        }
        OnCloseEditor();
    });
    m_editor->OnNowActivate.Connect(CDL, [menu]{ menu->setProperty("a_accept", true); menu->close(); });
    m_editor->OnApplyActivate.Connect(CDL, [menu]{ menu->setProperty("a_accept", true); menu->close(); });

    m_connectors.AddConnector<LocalPropertiesDateTimeConnector>(&m_editor->CurrentDateTime, ui->dateTimeEdit, &TimeShift);

//    WidgetAbstractButtonWrapper(ui->CalendarButton).SetControl(ButtonRole::DateTimePicker).SetIcon("Calendar");
}

WidgetsDateTimeLayout::~WidgetsDateTimeLayout()
{
    delete ui;
}

QLabel* WidgetsDateTimeLayout::label() const
{
    return ui->label;
}

WidgetsDateTimeWidget* WidgetsDateTimeLayout::popUp() const
{
    return m_editor;
}

WidgetsDateTimeEdit* WidgetsDateTimeLayout::dateTime() const
{
    return ui->dateTimeEdit;
}

QString WidgetsDateTimeLayout::title() const
{
    return ui->label->text();
}

bool WidgetsDateTimeLayout::isDateTime() const
{
    return m_editor->Mode == WidgetsDateTimeWidget::DateTime;
}

void WidgetsDateTimeLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsDateTimeLayout::setIsDateTime(const bool& dateTime)
{
    auto mode = dateTime ? WidgetsDateTimeWidget::DateTime : WidgetsDateTimeWidget::Date;
    m_editor->Mode = mode;
    ui->dateTimeEdit->Mode = mode;
}

