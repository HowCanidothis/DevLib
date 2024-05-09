#include "widgetsdoublespinboxlayout.h"
#include "ui_widgetsdoublespinboxlayout.h"

#include <QCheckBox>

#include <PropertiesModule/Ui/internal.hpp>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsDoubleSpinBoxLayout::WidgetsDoubleSpinBoxLayout(QWidget *parent)
    : Super(parent)
    , Disable(false)
    , ui(new Ui::WidgetsDoubleSpinBoxLayout)
    , m_checkbox(nullptr)
{
    ui->setupUi(this);
    WidgetWrapper(ui->spinbox).ConnectFocus(ui->label);
    setFocusProxy(ui->spinbox);
}

WidgetsDoubleSpinBoxLayout::~WidgetsDoubleSpinBoxLayout()
{
    delete ui;
}

bool WidgetsDoubleSpinBoxLayout::readOnly() const
{
    return ui->spinbox->isReadOnly();
}

QLabel* WidgetsDoubleSpinBoxLayout::label() const
{
    return ui->label;
}

QHBoxLayout* WidgetsDoubleSpinBoxLayout::layout() const
{
    return ui->horizontalLayout;
}

QHBoxLayout* WidgetsDoubleSpinBoxLayout::headerLayout() const
{
    return ui->horizontalLayout_2;
}

WidgetsDoubleSpinBoxWithCustomDisplay* WidgetsDoubleSpinBoxLayout::spinBox() const
{
    return ui->spinbox;
}

Qt::Orientation WidgetsDoubleSpinBoxLayout::orientation() const
{
    return ui->verticalLayout->direction() == QBoxLayout::TopToBottom ? Qt::Vertical : Qt::Horizontal;
}

void WidgetsDoubleSpinBoxLayout::setOrientation(Qt::Orientation orientation)
{
    if(this->orientation() != orientation) {
        if(orientation == Qt::Vertical) {
            ui->verticalLayout->setDirection(QBoxLayout::TopToBottom);
        } else {
            ui->verticalLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
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
        m_checkbox = new WidgetsLayoutComponent<QCheckBox>("checkbox");
        auto& connections = WidgetWrapper(m_checkbox->Widget).WidgetConnections();
        ui->horizontalLayout_2->insertWidget(0, m_checkbox->Widget);
        WidgetWrapper(ui->spinbox).WidgetEnablity().ConnectFrom(CDL, [](bool disable, bool check){
            return !disable && check;
        }, Disable, WidgetCheckBoxWrapper(m_checkbox->Widget).WidgetChecked()).MakeSafe(connections);
    }
}

void WidgetsDoubleSpinBoxLayout::ensureHasBox()
{
    if(m_lineEdit == nullptr) {
        m_lineEdit = new WidgetsLayoutComponent<QLineEdit>("lineedit");
        m_lineEdit->Widget->setReadOnly(true);
        ui->horizontalLayout->insertWidget(1, m_lineEdit->Widget);
    }
}

void WidgetsDoubleSpinBoxLayout::setReadOnly(bool readOnly)
{
    ui->spinbox->setReadOnly(readOnly);
}

bool WidgetsDoubleSpinBoxLayout::checked() const
{
    if(m_checkbox == nullptr) {
        return false;
    }
    return m_checkbox->Widget->isChecked();
}

void WidgetsDoubleSpinBoxLayout::setChecked(bool checked)
{
    ensureCheckable();
    m_checkbox->Widget->setChecked(checked);
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

bool WidgetsDoubleSpinBoxLayout::hasBox() const
{
    return m_lineEdit != nullptr;
}

void WidgetsDoubleSpinBoxLayout::setHasBox(bool hasBox)
{
    if(hasBox) {
        ensureHasBox();
    } else if(m_lineEdit != nullptr){
        m_lineEdit->Detach();
        m_lineEdit = nullptr;
    }
}

bool WidgetsDoubleSpinBoxLayout::disable() const
{
    return Disable;
}

void WidgetsDoubleSpinBoxLayout::setDisable(bool disable){
    Disable = disable;
}


