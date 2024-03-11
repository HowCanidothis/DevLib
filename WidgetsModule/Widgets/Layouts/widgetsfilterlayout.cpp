#include "widgetsfilterlayout.h"
#include "ui_widgetsfilterlayout.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsFilterLayout::WidgetsFilterLayout(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsFilterLayout)
{
    ui->setupUi(this);
    WidgetDoubleSpinBoxWrapper(ui->doubleSpinBox).WidgetReadOnly().ConnectFrom(CDL, FInverseBool, WidgetCheckBoxWrapper(ui->checkBox).WidgetChecked());
}

WidgetsFilterLayout::~WidgetsFilterLayout()
{
    delete ui;
}

QLabel* WidgetsFilterLayout::label() const
{
    return ui->label;
}

QCheckBox* WidgetsFilterLayout::checkBox() const
{
    return ui->checkBox;
}

WidgetsDoubleSpinBoxWithCustomDisplay* WidgetsFilterLayout::spinBox() const
{
    return ui->doubleSpinBox;
}

QString WidgetsFilterLayout::title() const
{
    return ui->label->text();
}

bool WidgetsFilterLayout::checked() const
{
    return ui->checkBox->isChecked();
}

void WidgetsFilterLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsFilterLayout::setChecked(const bool& checked)
{
    WidgetCheckBoxWrapper(ui->checkBox).WidgetChecked() = checked;
}
