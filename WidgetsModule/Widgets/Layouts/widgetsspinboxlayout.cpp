#include "widgetsspinboxlayout.h"
#include "ui_widgetsspinboxlayout.h"

WidgetsSpinBoxLayout::WidgetsSpinBoxLayout(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetsSpinBoxLayout)
{
    ui->setupUi(this);
}

WidgetsSpinBoxLayout::~WidgetsSpinBoxLayout()
{
    delete ui;
}

QLabel* WidgetsSpinBoxLayout::label()
{
    return ui->label;
}

QSpinBox* WidgetsSpinBoxLayout::spinBox()
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
