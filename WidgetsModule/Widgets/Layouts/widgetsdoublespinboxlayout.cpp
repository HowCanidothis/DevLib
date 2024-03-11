#include "widgetsdoublespinboxlayout.h"
#include "ui_widgetsdoublespinboxlayout.h"

#include <WidgetsModule/internal.hpp>
#include <PropertiesModule/Ui/internal.hpp>

WidgetsDoubleSpinBoxLayout::WidgetsDoubleSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDoubleSpinBoxLayout)
{
    ui->setupUi(this);
    ui->lineEdit->hide();
    WidgetWrapper(ui->doubleSpinBox).ConnectFocus(ui->label);
}

WidgetsDoubleSpinBoxLayout::~WidgetsDoubleSpinBoxLayout()
{
    delete ui;
}

bool WidgetsDoubleSpinBoxLayout::readOnly() const
{
    return ui->doubleSpinBox->isReadOnly();
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

void WidgetsDoubleSpinBoxLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsDoubleSpinBoxLayout::setReadOnly(bool readOnly)
{
    ui->doubleSpinBox->setReadOnly(readOnly);
}
