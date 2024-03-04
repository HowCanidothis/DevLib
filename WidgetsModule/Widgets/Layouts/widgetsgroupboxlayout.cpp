#include "widgetsgroupboxlayout.h"
#include "ui_widgetsgroupboxlayout.h"

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsGroupBoxLayout::WidgetsGroupBoxLayout(QWidget *parent) :
    Super(parent),
    ui(new Ui::WidgetsGroupBoxLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->groupBar).AddEventFilter([this](QObject*, QEvent* e) {
        switch (e->type()) {
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
            qApp->sendEvent(ui->groupIcon, e);
            break;
        default:
            break;
        }
        return false;
    });
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
    return true;
}

QString WidgetsGroupBoxLayout::title() const
{
    return ui->groupBar->text();
}

void WidgetsGroupBoxLayout::setTitle(const QString& title)
{
    ui->groupBar->setText(title);
}
