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

QLabel* WidgetsComboBoxLayout::label() const
{
    return ui->label;
}

QComboBox* WidgetsComboBoxLayout::comboBox() const
{
    return ui->comboBox;
}

QHBoxLayout* WidgetsComboBoxLayout::layout() const
{
    return ui->horizontalLayout;
}

QString WidgetsComboBoxLayout::title() const
{
    return ui->label->text();
}

void WidgetsComboBoxLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

bool WidgetsComboBoxLayout::editable() const
{
    return comboBox()->editable();
}

void WidgetsComboBoxLayout::setEditable(const bool& editable)
{
    comboBox()->setEditable(editable);
}
