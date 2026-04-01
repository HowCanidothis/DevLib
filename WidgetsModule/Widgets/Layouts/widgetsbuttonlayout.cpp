#include "widgetsbuttonlayout.h"
#include "ui_widgetsbuttonlayout.h"

#include <QPushButton>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsButtonLayout::WidgetsButtonLayout(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsButtonLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->pushButton).ConnectFocus(ui->label);
    setFocusProxy(ui->pushButton);
}

WidgetsButtonLayout::~WidgetsButtonLayout()
{
    delete ui;
}

Qt::Orientation WidgetsButtonLayout::orientation() const
{
    return ui->verticalLayout->direction() == QBoxLayout::TopToBottom ? Qt::Vertical : Qt::Horizontal;
}

void WidgetsButtonLayout::setOrientation(Qt::Orientation orientation)
{
    if(this->orientation() != orientation) {
        if(orientation == Qt::Vertical) {
            ui->verticalLayout->setDirection(QBoxLayout::TopToBottom);
        } else {
            ui->verticalLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
}

QLabel* WidgetsButtonLayout::label() const
{
    return ui->label;
}

QPushButton* WidgetsButtonLayout::pushButton() const
{
    return ui->pushButton;
}

QHBoxLayout* WidgetsButtonLayout::valueLayout() const
{
    return ui->horizontalLayout;
}

QHBoxLayout* WidgetsButtonLayout::headerLayout() const
{
    return ui->horizontalLayout_2;
}

void WidgetsButtonLayout::setHasButton(bool has)
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

QString WidgetsButtonLayout::title() const
{
    return ui->label->text();
}

void WidgetsButtonLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}
