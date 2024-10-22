#include "widgetstabbarlayout.h"
#include "ui_widgetstabbarlayout.h"

#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsTabBarLayout::WidgetsTabBarLayout(QWidget *parent)
    : QFrame(parent)
    , Opened(true)
    , ui(new Ui::WidgetsTabBarLayout)
    , m_prevIndex(-1)
    , m_currentIndex(-1)
    , m_icon(nullptr)
    , m_collapsable(false)
{
    ui->setupUi(this);

    m_currentIndex.Connect(CDL, [this](qint32 index){
        qint32 cindex = m_prevIndex;
        if(index >= 0 && index < m_buttons.size()) {
            WidgetAbstractButtonWrapper(m_buttons.at(index)).SetChecked(true);
            m_views.at(index)->show();
        }
        m_prevIndex = index;
        if(cindex == index) {
            return;
        }
        if(cindex >= 0 && cindex < m_buttons.size()) {
            WidgetAbstractButtonWrapper(m_buttons.at(cindex)).SetChecked(false);
            m_views.at(cindex)->hide();
        }
        if(index != -1) {
            Opened = true;
        } else {
            Opened = false;
        }
        emit currentIndexChanged(index);
    });

    setFocusPolicy(Qt::StrongFocus);
    WidgetWrapper(this).AddEventFilter([this](QObject*, QEvent* e){
        if(e->type() == QEvent::KeyPress){
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            switch (keyEvent->key()) {
            case Qt::Key_Right:
                m_currentIndex = m_currentIndex + 1 == m_buttons.size() ? 0 : m_currentIndex + 1;
                return true;
            case Qt::Key_Left:
                m_currentIndex = m_currentIndex.Native() ? m_currentIndex - 1 : m_buttons.size() - 1;
                return true;
            default: break;
            }
        }
        return false;
    });
}

bool WidgetsTabBarLayout::collapsable() const
{
    return m_collapsable;
}

void WidgetsTabBarLayout::setCollapsable(bool collapsable)
{
    m_collapsable = collapsable;
    if(collapsable) {
        if(m_icon == nullptr) {
            m_icon = new QLabel();
            ui->horizontalLayout->addWidget(m_icon);
            m_icon->setObjectName("groupIcon");
            Opened = m_currentIndex != -1;
            Opened.ConnectAndCall(CDL, [this](bool opened) {
                WidgetWrapper(m_icon).ApplyStyleProperty("a_opened", opened);
                if(!opened) {
                    m_currentIndex = -1;
                }
            });
            WidgetTabBarLayoutWrapper(this).AddCollapsing();
            WidgetTabBarLayoutWrapper(this).AddCollapsingDispatcher(m_currentIndex.OnChanged);
            auto updateChecked = [this]{
                if(Opened) {
                    Opened = false;
                }
            };
            WidgetWrapper(m_icon).SetOnClicked(updateChecked);
        }
        m_icon->show();
    } else if(m_icon != nullptr){
        m_icon->hide();
    }
}

WidgetsTabBarLayout::~WidgetsTabBarLayout()
{
    delete ui;
}

const QVector<QPushButton*>& WidgetsTabBarLayout::buttons() const
{
    return m_buttons;
}

QWidget* WidgetsTabBarLayout::widgetAt(qint32 index) const
{
    if(index >= 0 && index < m_views.size()) {
        return m_views.at(index);
    }
    return nullptr;
}

QWidget* WidgetsTabBarLayout::currentWidget() const
{
    return widgetAt(m_currentIndex);
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
    for(auto* w : m_views) {
        auto* l = w->layout();
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
    auto* widget = currentWidget();
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
    b->setFocusPolicy(Qt::NoFocus);
    b->setCheckable(true);
#ifdef QT_PLUGIN
    b->setObjectName("__qt__passive_button_" + QString::number(index));
#else
    b->setObjectName(QString::number(index));
#endif
    b->setProperty("a_page", QVariant::fromValue(page));
    b->setText(page->windowTitle());
    b->setProperty("a_tab_button", true);

    connect(page, &QWidget::windowTitleChanged, [this, b](const QString& title){
        if(b->text() == title) {
            return;
        }
        b->setText(title);
        emit pageTitleChanged(title);
    });

    WidgetAbstractButtonWrapper(b).SetControl(ButtonRole::Tab).WidgetChecked().Connect(CDL, [this, b](bool checked){
        if(checked) {
            m_currentIndex = m_buttons.indexOf(b);
        }
    });
    WidgetAbstractButtonWrapper(b).SetOnClicked([this, b]{
        m_currentIndex.SetValueForceInvoke(m_buttons.indexOf(b));
    });
    m_buttons.insert(index, b);
    page->setProperty("a_is_page", true);
    ui->horizontalLayout->insertWidget(index, b);
    QVBoxLayout* l = reinterpret_cast<QVBoxLayout*>(ui->contentWidget->layout());
    l->insertWidget(index, page);
    m_views.insert(index, page);
    page->hide();
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
    delete m_buttons.takeAt(index);
    m_views.takeAt(index);
    m_currentIndex = index - 1;
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

void WidgetsTabBarLayout::SetIcons(const QVector<const Name*>& icons)
{
    Q_ASSERT(icons.size() == m_buttons.size());
    adapters::Foreach([](QPushButton* button, const Name* icon){
        WidgetPushButtonWrapper(button).SetIcon(*icon);
    }, m_buttons, icons);
}
