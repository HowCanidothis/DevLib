#include "widgetsspinboxlayout.h"
#include "ui_widgetsspinboxlayout.h"

#include <QCheckBox>
#include <WidgetsModule/internal.hpp>

WidgetsSpinBoxLayout::WidgetsSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsSpinBoxLayout)
    , m_checkbox(nullptr)
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

void WidgetsSpinBoxLayout::ensureCheckable()
{
    if(m_checkbox == nullptr) {
        m_checkbox = new CheckBoxComponent();
        ui->horizontalLayout->insertWidget(0, m_checkbox->Checkbox);
        WidgetWrapper(ui->spinBox).WidgetEnablity().ConnectFrom(CDL, WidgetCheckBoxWrapper(m_checkbox->Checkbox).WidgetChecked()).MakeSafe(WidgetWrapper(m_checkbox->Checkbox).WidgetConnections());
    }
}

bool WidgetsSpinBoxLayout::checked() const
{
    if(m_checkbox == nullptr) {
        return false;
    }
    return m_checkbox->Checkbox->isChecked();
}

void WidgetsSpinBoxLayout::setChecked(bool checked)
{
    ensureCheckable();
    m_checkbox->Checkbox->setChecked(checked);
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

WidgetsSpinBoxLayout::CheckBoxComponent::CheckBoxComponent()
    : Checkbox(new QCheckBox())
{
}

void WidgetsSpinBoxLayout::CheckBoxComponent::Detach()
{
    delete Checkbox;
}
