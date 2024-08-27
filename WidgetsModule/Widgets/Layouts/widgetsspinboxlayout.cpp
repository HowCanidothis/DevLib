#include "widgetsspinboxlayout.h"
#include "ui_widgetsspinboxlayout.h"

#include <QCheckBox>
#include <QLineEdit>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsSpinBoxLayout::WidgetsSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsSpinBoxLayout)
    , m_checkbox(nullptr)
{
    ui->setupUi(this);
    WidgetWrapper(ui->spinbox).ConnectFocus(ui->label);
    setFocusProxy(ui->spinbox);
}

QHBoxLayout* WidgetsSpinBoxLayout::layout() const
{
    return ui->horizontalLayout_2;
}

bool WidgetsSpinBoxLayout::hasBox() const
{
    return m_lineEdit != nullptr;
}

void WidgetsSpinBoxLayout::setHasBox(bool hasBox)
{
    if(hasBox) {
        ensureHasBox();
    } else if(m_lineEdit != nullptr){
        m_lineEdit->Detach();
        m_lineEdit = nullptr;
    }
}

void WidgetsSpinBoxLayout::ensureHasBox()
{
    if(m_lineEdit == nullptr) {
        m_lineEdit = new WidgetsLayoutComponent<QLineEdit>("lineedit");
        m_lineEdit->Widget->setReadOnly(true);
        ui->horizontalLayout_2->insertWidget(1, m_lineEdit->Widget);
    }
}

WidgetsSpinBoxLayout::~WidgetsSpinBoxLayout()
{
    delete ui;
}

Qt::Orientation WidgetsSpinBoxLayout::orientation() const
{
    return ui->verticalLayout->direction() == QBoxLayout::TopToBottom ? Qt::Vertical : Qt::Horizontal;
}

void WidgetsSpinBoxLayout::setOrientation(Qt::Orientation orientation)
{
    if(this->orientation() != orientation) {
        if(orientation == Qt::Vertical) {
            ui->verticalLayout->setDirection(QBoxLayout::TopToBottom);
        } else {
            ui->verticalLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
}


QLabel* WidgetsSpinBoxLayout::label() const
{
    return ui->label;
}

WidgetsSpinBoxWithCustomDisplay* WidgetsSpinBoxLayout::spinBox() const
{
    return ui->spinbox;
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
    return ui->spinbox->isReadOnly();
}

void WidgetsSpinBoxLayout::setReadOnly(bool readOnly)
{
    ui->spinbox->setReadOnly(readOnly);
}

void WidgetsSpinBoxLayout::ensureCheckable()
{
    if(m_checkbox == nullptr) {
        m_checkbox = new WidgetsLayoutComponent<QCheckBox>("checkbox");
        ui->horizontalLayout->insertWidget(0, m_checkbox->Widget);
        WidgetWrapper(ui->spinbox).WidgetEnablity().ConnectFrom(CDL, WidgetCheckBoxWrapper(m_checkbox->Widget).WidgetChecked()).MakeSafe(WidgetWrapper(m_checkbox->Widget).WidgetConnections());
    }
}

bool WidgetsSpinBoxLayout::checked() const
{
    if(m_checkbox == nullptr) {
        return false;
    }
    return m_checkbox->Widget->isChecked();
}

void WidgetsSpinBoxLayout::setChecked(bool checked)
{
    ensureCheckable();
    m_checkbox->Widget->setChecked(checked);
}

bool WidgetsSpinBoxLayout::checkable() const
{
    return m_checkbox != nullptr;
}

void WidgetsSpinBoxLayout::setCheckable(bool checkable)
{
    if(checkable) {
        ensureCheckable();
    } else if(m_checkbox != nullptr){
        m_checkbox->Detach();
        m_checkbox = nullptr;
    }
}
