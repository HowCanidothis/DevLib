#include "widgetsgroupboxlayout.h"
#include "ui_widgetsgroupboxlayout.h"

WidgetsGroupBoxLayout::WidgetsGroupBoxLayout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetsGroupBoxLayout)
{
    ui->setupUi(this);
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
    return ui->pushButton->text();
}

void WidgetsGroupBoxLayout::setTitle(const QString& title)
{
    ui->pushButton->setText(title);
}
