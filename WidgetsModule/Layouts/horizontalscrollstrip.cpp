#include "horizontalscrollstrip.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>

#include <WidgetsModule/internal.hpp>

HorizontalScrollStrip::HorizontalScrollStrip(QWidget* parent)
    : Super(parent)
    , m_leftButton(new QPushButton(this))
    , m_rightButton(new QPushButton(this))
    , m_scrollArea(new QScrollArea(this))
    , m_content(new QWidget())
    , m_layout(new QHBoxLayout(m_content))
    , m_autoHideArrows(true)
{
    Spacing.EditSilent() = 8;

    // Стрелки: только функция, без размеров и стилей.
    m_leftButton->setObjectName(QStringLiteral("btnPrevious"));
    m_rightButton->setObjectName(QStringLiteral("btnNext"));
    m_leftButton->setAutoRepeat(true);
    m_rightButton->setAutoRepeat(true);
    m_leftButton->setFocusPolicy(Qt::NoFocus);
    m_rightButton->setFocusPolicy(Qt::NoFocus);

    // Контент: размер строго по sizeHint раскладки.
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(Spacing);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    // Область прокрутки: скроллбары скрыты.
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_scrollArea->setWidget(m_content);
    m_content->installEventFilter(this);

    auto* rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(m_leftButton);
    rootLayout->addWidget(m_scrollArea);
    rootLayout->addWidget(m_rightButton);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Qt-сигналы.
    connect(m_leftButton, &QPushButton::clicked, this, [this]{ scrollByStep(-1); });
    connect(m_rightButton, &QPushButton::clicked, this, [this]{ scrollByStep(1); });
    QScrollBar* bar = m_scrollArea->horizontalScrollBar();
    connect(bar, &QScrollBar::rangeChanged, this, [this]{ updateArrowsEnabled(); });
    connect(bar, &QScrollBar::valueChanged, this, [this]{ updateArrowsEnabled(); });

    // Свойство DevLib: как в FlowWidget (Subscribe без MakeSafe, источник и подписчик - один объект).
    Spacing.Subscribe([this]{
        m_layout->setSpacing(Spacing);
    });

    updateArrowsVisibility();
}

void HorizontalScrollStrip::Clear()
{
    QLayoutItem* item = m_layout->takeAt(0);
    while(item != nullptr) {
        QWidget* widget = item->widget();
        if(widget != nullptr) {
            widget->hide();
            widget->deleteLater();
        }
        delete item;
        item = m_layout->takeAt(0);
    }
    updateArrowsVisibility();
}

void HorizontalScrollStrip::AddWidget(QWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    m_layout->addWidget(widget);
}

void HorizontalScrollStrip::InsertWidget(qint32 index, QWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    Q_ASSERT(index >= 0 && index <= m_layout->count());
    m_layout->insertWidget(index, widget);
}

void HorizontalScrollStrip::RemoveWidget(QWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    Q_ASSERT(m_layout->indexOf(widget) != -1);
    m_layout->removeWidget(widget);
    widget->hide();
    widget->setParent(nullptr);
}

void HorizontalScrollStrip::SetAutoHideArrows(bool autoHide)
{
    m_autoHideArrows = autoHide;
    updateArrowsVisibility();
}

void HorizontalScrollStrip::EnableWheelScrolling()
{
    WidgetScrollAreaWrapper(m_scrollArea).AddScrollByWheel(Qt::Horizontal);
}

QSize HorizontalScrollStrip::sizeHint() const
{
    const QSize contentHint = m_content->sizeHint();
    const qint32 arrowsHeight = qMax(m_leftButton->sizeHint().height(), m_rightButton->sizeHint().height());
    return QSize(contentHint.width(), qMax(contentHint.height(), arrowsHeight));
}

QSize HorizontalScrollStrip::minimumSizeHint() const
{
    const QSize hint = sizeHint();
    const qint32 arrowsWidth = m_leftButton->sizeHint().width() + m_rightButton->sizeHint().width();
    return QSize(qMin(hint.width(), arrowsWidth), hint.height());
}

bool HorizontalScrollStrip::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == m_content && event->type() == QEvent::Resize) {
        updateGeometry();
        updateArrowsVisibility();
    }
    return Super::eventFilter(watched, event);
}

void HorizontalScrollStrip::resizeEvent(QResizeEvent* event)
{
    Super::resizeEvent(event);
    updateArrowsVisibility();
}

void HorizontalScrollStrip::showEvent(QShowEvent* event)
{
    Super::showEvent(event);
    updateArrowsVisibility();
}

void HorizontalScrollStrip::scrollByStep(qint32 direction)
{
    // Шаг - половина видимой ширины; QScrollBar сам ограничит диапазон.
    QScrollBar* bar = m_scrollArea->horizontalScrollBar();
    const qint32 step = qMax(1, m_scrollArea->viewport()->width() / 2);
    bar->setValue(bar->value() + direction * step);
}

bool HorizontalScrollStrip::isOverflow() const
{
    // Сравниваем с шириной самой полосы, а не с диапазоном скроллбара: показ стрелок
    // сужает viewport, и без этого решение мигало бы на границе.
    return m_content->sizeHint().width() > contentsRect().width();
}

void HorizontalScrollStrip::updateArrowsVisibility()
{
    const bool visible = m_autoHideArrows ? isOverflow() : true;
    m_leftButton->setVisible(visible);
    m_rightButton->setVisible(visible);
    updateArrowsEnabled();
}

void HorizontalScrollStrip::updateArrowsEnabled()
{
    const QScrollBar* bar = m_scrollArea->horizontalScrollBar();
    m_leftButton->setEnabled(bar->value() > bar->minimum());
    m_rightButton->setEnabled(bar->value() < bar->maximum());
}
