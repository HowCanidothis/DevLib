#include "widgetsspinboxlayout.h"
#include "ui_widgetsspinboxlayout.h"

WidgetsSpinBoxLayout::WidgetsSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsSpinBoxLayout)
{
    ui->setupUi(this);
}

WidgetsSpinBoxLayout::~WidgetsSpinBoxLayout()
{
    delete ui;
}

QLabel* WidgetsSpinBoxLayout::label() const
{
    return ui->label;
}

WidgetsSpinBoxWithCustomDisplay* WidgetsSpinBoxLayout::spinBox() const
{
    return ui->spinBox;
}

QString WidgetsSpinBoxLayout::title() const
{
    return ui->label->text();
}

void WidgetsSpinBoxLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}
