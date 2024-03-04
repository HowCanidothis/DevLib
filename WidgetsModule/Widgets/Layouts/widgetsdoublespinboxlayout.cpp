#include "widgetsdoublespinboxlayout.h"
#include "ui_widgetsdoublespinboxlayout.h"

WidgetsDoubleSpinBoxLayout::WidgetsDoubleSpinBoxLayout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetsDoubleSpinBoxLayout)
{
    ui->setupUi(this);
}

WidgetsDoubleSpinBoxLayout::~WidgetsDoubleSpinBoxLayout()
{
    delete ui;
}

QLabel* WidgetsDoubleSpinBoxLayout::label()
{
    return ui->label;
}

QLineEdit* WidgetsDoubleSpinBoxLayout::lineEdit()
{
    return ui->lineEdit;
}

WidgetsDoubleSpinBoxWithCustomDisplay* WidgetsDoubleSpinBoxLayout::spinBox()
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
