#include "widgetscomboboxlayout.h"
#include "ui_widgetscomboboxlayout.h"

#include <QPushButton>
#include <QAbstractItemView>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsComboBoxLayout::WidgetsComboBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsComboBoxLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->combobox->view()).ConnectFocus(ui->label);
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

void WidgetsComboBoxLayout::setHasButton(bool has)
{
    if(has) {
        if(m_button == nullptr) {
            m_button = new WidgetsLayoutComponent<QPushButton>("button");
            WidgetAbstractButtonWrapper(m_button->Widget).SetControl(ButtonRole::Icon);
            ui->horizontalLayout_2->insertWidget(0, m_button->Widget);
        }
    } else if(m_button != nullptr){
        m_button->Detach();
        m_button = nullptr;
    }
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
