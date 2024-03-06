#include "widgetstabbarlayout.h"
#include "ui_widgetstabbarlayout.h"

#include <QPushButton>
#include <WidgetsModule/internal.hpp>

WidgetsTabBarLayout::WidgetsTabBarLayout(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WidgetsTabBarLayout)
{
    ui->setupUi(this);

    m_currentIndex.Connect(CDL, [this](qint32 index){
        qint32 cindex = ui->stackedWidget->currentIndex();
        if(index >= 0 && index < m_buttons.size()) {
            WidgetAbstractButtonWrapper(m_buttons.at(index)).SetChecked(true);
        }
        if(cindex == index) {
            return;
        }
        if(cindex >= 0 && cindex < m_buttons.size()) {
            WidgetAbstractButtonWrapper(m_buttons.at(cindex)).SetChecked(false);
        }
        ui->stackedWidget->setCurrentIndex(index);
        emit currentIndexChanged(index);
    });
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

qint32 WidgetsTabBarLayout::currentIndex() const
{
    return m_currentIndex;
}

qint32 WidgetsTabBarLayout::buttonsGap() const
{
    return ui->horizontalLayout->spacing();
}

qint32 WidgetsTabBarLayout::gap() const
{
    return ui->verticalLayout->spacing();
}

QString WidgetsTabBarLayout::title() const
{
    qint32 cindex = m_currentIndex.Native();
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
    auto* widget = ui->stackedWidget->currentWidget();
    if(widget != nullptr) {
        widget->setWindowTitle(text);
        emit windowTitleChanged(text);
    }
}

void WidgetsTabBarLayout::addPage(QWidget* page)
{
    insertPage(m_buttons.size(), page);
}

void WidgetsTabBarLayout::insertPage(int index, QWidget* page)
{
    auto* b = new QPushButton();
    b->setCheckable(true);
#ifdef QT_PLUGIN
    b->setObjectName("__qt__passive_button_" + QString::number(index));
#else
    b->setObjectName(QString::number(index));
#endif
    b->setProperty("a_page", QVariant::fromValue(page));
    b->setText(page->windowTitle());

    connect(page, &QWidget::windowTitleChanged, [this, b](const QString& title){
        if(b->text() == title) {
            return;
        }
        b->setText(title);
        emit pageTitleChanged(title);
    });

    WidgetAbstractButtonWrapper(b).SetOnClicked([this, b]{
        m_currentIndex.SetValueForceInvoke(m_buttons.indexOf(b));
    }).SetControl(ButtonRole::Tab);
    m_buttons.insert(index, b);
    page->setProperty("a_is_page", true);
    ui->horizontalLayout->insertWidget(index, b);
    ui->stackedWidget->insertWidget(index, page);
    if(index == m_currentIndex) {
        m_currentIndex.Invoke();
    }
#ifdef QT_PLUGIN
    for(auto* b : adapters::range(m_buttons, index + 1)) {
        b->setObjectName("__qt__passive_button_" + QString::number(++index));
    }
#endif
}

void WidgetsTabBarLayout::removePage(int index)
{
    auto* b = m_buttons.takeAt(index);
    auto* w = b->property("a_page").value<QWidget*>();
    ui->stackedWidget->removeWidget(w);
    delete b;
#ifdef QT_PLUGIN
    for(auto* b : adapters::range(m_buttons, index)) {
        b->setObjectName("__qt__passive_button_" + QString::number(++index));
    }
#endif
}

void WidgetsTabBarLayout::setCurrentIndex(qint32 index)
{
    m_currentIndex = index;
}
