#include "widgetsgroupboxlayout.h"
#include "ui_widgetsgroupboxlayout.h"

#include <QLabel>

#include <PropertiesModule/Ui/internal.hpp>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsGroupBoxLayout::WidgetsGroupBoxLayout(QWidget *parent)
    : Super(parent)
    , Opened(true)
    , ui(new Ui::WidgetsGroupBoxLayout)
    , m_icon(nullptr)
    , m_collapsable(false)
{
    ui->setupUi(this);
    ui->groupBar->hide();
    WidgetWrapper(ui->widget).WidgetVisibility().ConnectFrom(CDL, Opened).MakeSafe(WidgetWrapper(ui->widget).WidgetConnections());
    setFocusProxy(ui->widget);
}

WidgetsGroupBoxLayout::~WidgetsGroupBoxLayout()
{
    delete ui;
}

QWidget* WidgetsGroupBoxLayout::widget() const
{
    return ui->widget;
}

bool WidgetsGroupBoxLayout::setWidget(QWidget* widget)
{
    delete ui->widget;
    ui->verticalLayout->addWidget(widget);
    ui->widget = widget;
    ui->widget->setProperty("a_contentWidget", true);
    WidgetWrapper(ui->widget).WidgetVisibility().ConnectFrom(CDL, Opened).MakeSafe(WidgetWrapper(ui->widget).WidgetConnections());
    setFocusProxy(widget);
    return true;
}

bool WidgetsGroupBoxLayout::collapsable() const
{
    return m_collapsable;
}

void WidgetsGroupBoxLayout::setCollapsable(bool collapsable)
{
    m_collapsable = collapsable;
    if(collapsable) {
        if(m_icon == nullptr) {
            m_icon = new QLabel(ui->groupBar);
            m_icon->setObjectName("groupIcon");
            m_icon->setAttribute(Qt::WA_TransparentForMouseEvents);
            Opened.ConnectAndCall(CDL, [this](bool opened) {
                WidgetWrapper(m_icon).ApplyStyleProperty("a_opened", opened);
            });
            WidgetWrapper(m_icon).LocateToParent(DescWidgetsLocationAttachmentParams(QuadTreeF::Location_MiddleRight));
            WidgetGroupboxLayoutWrapper(this).AddCollapsing();
            auto updateChecked = [this]{
                Opened = !Opened;
            };
            WidgetAbstractButtonWrapper(ui->groupBar).SetOnClicked(updateChecked);
        }
        m_icon->show();
        ui->groupBar->setFocusPolicy(Qt::StrongFocus);
    } else if(m_icon != nullptr){
        m_icon->hide();
        ui->groupBar->setFocusPolicy(Qt::NoFocus);
    }
}

qint32 WidgetsGroupBoxLayout::gap() const
{
    return ui->verticalLayout->spacing();
}

void WidgetsGroupBoxLayout::setGap(qint32 gap)
{
    if(ui->widget->layout() != nullptr) {
        ui->widget->layout()->setSpacing(gap);
    }
    ui->verticalLayout->setSpacing(gap);

}

QString WidgetsGroupBoxLayout::title() const
{
    return ui->groupBar->text();
}

void WidgetsGroupBoxLayout::setTitle(const QString& title)
{
    ui->groupBar->setText(title);
    ui->groupBar->setVisible(!title.isEmpty());
    if(title.isEmpty()) {
        return;
    }
}
