#include "widgetsdoublespinboxlayout.h"
#include "ui_widgetsdoublespinboxlayout.h"

#include <QCheckBox>

#include <WidgetsModule/internal.hpp>
#include <PropertiesModule/Ui/internal.hpp>

WidgetsDoubleSpinBoxLayout::WidgetsDoubleSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsDoubleSpinBoxLayout)
    , m_checkbox(nullptr)
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

void WidgetsDoubleSpinBoxLayout::ensureCheckable()
{
    if(m_checkbox == nullptr) {
        m_checkbox = new CheckBoxComponent();
        ui->horizontalLayout_2->insertWidget(0, m_checkbox->Checkbox);
        WidgetWrapper(ui->doubleSpinBox).WidgetEnablity().ConnectFrom(CDL, WidgetCheckBoxWrapper(m_checkbox->Checkbox).WidgetChecked()).MakeSafe(WidgetWrapper(m_checkbox->Checkbox).WidgetConnections());
    }
}

void WidgetsDoubleSpinBoxLayout::setReadOnly(bool readOnly)
{
    ui->doubleSpinBox->setReadOnly(readOnly);
}

bool WidgetsDoubleSpinBoxLayout::checked() const
{
    if(m_checkbox == nullptr) {
        return false;
    }
    return m_checkbox->Checkbox->isChecked();
}

void WidgetsDoubleSpinBoxLayout::setChecked(bool checked)
{
    ensureCheckable();
    m_checkbox->Checkbox->setChecked(checked);
}

bool WidgetsDoubleSpinBoxLayout::checkable() const
{
    return m_checkbox != nullptr;
}

void WidgetsDoubleSpinBoxLayout::setCheckable(bool checkable)
{
    if(checkable) {
        ensureCheckable();
    } else if(m_checkbox != nullptr){
        m_checkbox->Detach();
        m_checkbox = nullptr;
    }
}

WidgetsDoubleSpinBoxLayout::CheckBoxComponent::CheckBoxComponent()
    : Checkbox(new QCheckBox())
{
}

void WidgetsDoubleSpinBoxLayout::CheckBoxComponent::Detach()
{
    delete Checkbox;
}
