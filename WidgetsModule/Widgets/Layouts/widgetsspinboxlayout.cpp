#include "widgetsspinboxlayout.h"
#include "ui_widgetsspinboxlayout.h"

#include <WidgetsModule/internal.hpp>

WidgetsSpinBoxLayout::WidgetsSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsSpinBoxLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->spinBox).ConnectFocus(ui->label);
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

bool WidgetsSpinBoxLayout::readOnly() const
{
    return ui->spinBox->isReadOnly();
}

void WidgetsSpinBoxLayout::setReadOnly(bool readOnly)
{
    ui->spinBox->setReadOnly(readOnly);
}
