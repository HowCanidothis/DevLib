#include "widgetstabbarlayout.h"
#include "ui_widgetstabbarlayout.h"

#include <QPushButton>
#include <WidgetsModule/internal.hpp>

WidgetsTabBarLayout::WidgetsTabBarLayout(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WidgetsTabBarLayout)
{
    ui->setupUi(this);
}

WidgetsTabBarLayout::~WidgetsTabBarLayout()
{
    delete ui;
}

QStackedWidget* WidgetsTabBarLayout::widget() const
{
    return ui->stackedWidget;
}

const QVector<QPushButton*>& WidgetsTabBarLayout::buttons() const
{
    return m_buttons;
}

qint32 WidgetsTabBarLayout::buttonsGap() const
{
    return ui->horizontalLayout->spacing();
}

qint32 WidgetsTabBarLayout::currentIndex() const
{
    return ui->stackedWidget->currentIndex();
}

qint32 WidgetsTabBarLayout::gap() const
{
    return ui->verticalLayout->spacing();
}

QString WidgetsTabBarLayout::title() const
{
    qint32 cindex = currentIndex();
    if(cindex >= 0 && cindex < m_buttons.size()) {
        return m_buttons.at(cindex)->text();
    }
    return QString();
}

void WidgetsTabBarLayout::setGap(qint32 gap)
{
    for(qint32 i(0); i < ui->stackedWidget->count(); ++i) {
        auto* l = ui->stackedWidget->widget(i)->layout();
        if(l != nullptr) {
            l->setSpacing(gap);
        }
    }
    ui->verticalLayout->setSpacing(gap);
}

void WidgetsTabBarLayout::setButtonsGap(qint32 gap)
{
    ui->horizontalLayout->setSpacing(gap);
}

void WidgetsTabBarLayout::setTitle(const QString& text)
{
    qint32 cindex = currentIndex();
    if(cindex >= 0 && cindex < m_buttons.size()) {
        m_buttons.at(cindex)->setText(text);
    }
}

void WidgetsTabBarLayout::addPage(QWidget* page)
{
    auto* b = new QPushButton();
    b->setCheckable(true);
    b->setObjectName("__qt__passive_button_" + QString::number(m_buttons.size()));
    b->setProperty("a_page", QVariant::fromValue(page));
    WidgetAbstractButtonWrapper(b).SetOnClicked([this, b]{
        setCurrentIndex(m_buttons.indexOf(b));
    });
    m_buttons.append(b);
    ui->horizontalLayout->addWidget(b);
    ui->stackedWidget->addWidget(page);

    setCurrentIndex(m_buttons.size() - 1);
}

void WidgetsTabBarLayout::insertPage(int index, QWidget* page)
{
    auto* b = new QPushButton();
    b->setCheckable(true);
    b->setObjectName("__qt__passive_button_" + QString::number(index));
    b->setProperty("a_page", QVariant::fromValue(page));
    WidgetAbstractButtonWrapper(b).SetOnClicked([this, b]{
        setCurrentIndex(m_buttons.indexOf(b));
    });
    m_buttons.insert(index, b);
    ui->horizontalLayout->insertWidget(index, b);
    ui->stackedWidget->insertWidget(index, page);

    setCurrentIndex(index);

    for(auto* b : adapters::range(m_buttons, index + 1)) {
        b->setObjectName("__qt__passive_button_" + QString::number(++index));
    }
}

void WidgetsTabBarLayout::removePage(int index)
{
    auto* b = m_buttons.takeAt(index);
    auto* w = b->property("a_page").value<QWidget*>();
    ui->stackedWidget->removeWidget(w);
    delete b;
    for(auto* b : adapters::range(m_buttons, index)) {
        b->setObjectName("__qt__passive_button_" + QString::number(++index));
    }
    setCurrentIndex(currentIndex());
}

void WidgetsTabBarLayout::setCurrentIndex(int index)
{
    qint32 cindex = currentIndex();
    if(cindex >= 0 && cindex < m_buttons.size()) {
        WidgetAbstractButtonWrapper(m_buttons.at(cindex)).SetChecked(false);
    }
    if(index >= 0 && index < m_buttons.size()) {
        WidgetAbstractButtonWrapper(m_buttons.at(index)).SetChecked(true);
    }
    ui->stackedWidget->setCurrentIndex(index);
}
