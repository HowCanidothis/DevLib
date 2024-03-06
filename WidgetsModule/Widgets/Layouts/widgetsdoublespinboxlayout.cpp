#include "widgetsdoublespinboxlayout.h"
#include "ui_widgetsdoublespinboxlayout.h"

WidgetsDoubleSpinBoxLayout::WidgetsDoubleSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDoubleSpinBoxLayout)
{
    ui->setupUi(this);
}

WidgetsDoubleSpinBoxLayout::~WidgetsDoubleSpinBoxLayout()
{
    delete ui;
}

QLabel* WidgetsDoubleSpinBoxLayout::label() const
{
    return ui->label;
}

QLineEdit* WidgetsDoubleSpinBoxLayout::lineEdit() const
{
    return ui->lineEdit;
}

QHBoxLayout* WidgetsDoubleSpinBoxLayout::layout() const
{
    return ui->horizontalLayout;
}

WidgetsDoubleSpinBoxWithCustomDisplay* WidgetsDoubleSpinBoxLayout::spinBox() const
{
    return ui->doubleSpinBox;
}

QString WidgetsDoubleSpinBoxLayout::title() const
{
    return ui->label->text();
}

bool WidgetsDoubleSpinBoxLayout::measurement() const
{
    return ui->lineEdit->isVisible();
}

void WidgetsDoubleSpinBoxLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsDoubleSpinBoxLayout::setMeasurement(const bool& measurement)
{
    ui->lineEdit->setVisible(measurement);
}
