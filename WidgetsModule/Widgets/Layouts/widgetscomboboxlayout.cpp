#include "widgetscomboboxlayout.h"
#include "ui_widgetscomboboxlayout.h"

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsComboBoxLayout::WidgetsComboBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsComboBoxLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->combobox->view()).ConnectFocus(ui->label, ui->combobox);
    setFocusProxy(ui->combobox);
}

WidgetsComboBoxLayout::~WidgetsComboBoxLayout()
{
    delete ui;
}

Qt::Orientation WidgetsComboBoxLayout::orientation() const
{
    return ui->verticalLayout->direction() == QBoxLayout::TopToBottom ? Qt::Vertical : Qt::Horizontal;
}

void WidgetsComboBoxLayout::setOrientation(Qt::Orientation orientation)
{
    if(this->orientation() != orientation) {
        if(orientation == Qt::Vertical) {
            ui->verticalLayout->setDirection(QBoxLayout::TopToBottom);
        } else {
            ui->verticalLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
}

QLabel* WidgetsComboBoxLayout::label() const
{
    return ui->label;
}

QLineEdit* WidgetsComboBoxLayout::lineEdit() const
{
    return ui->combobox->lineEdit();
}

QComboBox* WidgetsComboBoxLayout::comboBox() const
{
    return ui->combobox;
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
    return comboBox()->isEditable();
}

void WidgetsComboBoxLayout::setEditable(const bool& editable)
{
    comboBox()->setEditable(editable);
}
