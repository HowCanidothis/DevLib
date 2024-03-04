#include "widgetscomboboxlayout.h"
#include "ui_widgetscomboboxlayout.h"

WidgetsComboBoxLayout::WidgetsComboBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsComboBoxLayout)
{
    ui->setupUi(this);
}

WidgetsComboBoxLayout::~WidgetsComboBoxLayout()
{
    delete ui;
}

QLabel* WidgetsComboBoxLayout::label()
{
    return ui->label;
}

QComboBox* WidgetsComboBoxLayout::comboBox()
{
    return ui->comboBox;
}

QString WidgetsComboBoxLayout::title() const
{
    return ui->label->text();
}

void WidgetsComboBoxLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}
