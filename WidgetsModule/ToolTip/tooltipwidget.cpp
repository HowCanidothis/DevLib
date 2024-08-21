#include "tooltipwidget.h"

#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>

#include "WidgetsModule/Utils/widgethelpers.h"

ToolTipWidget::ToolTipWidget(QWidget* parent)
    : Super(parent)
    , m_content(nullptr)
{
    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0,0,0,0);
    layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

    OffsetFromTarget.Subscribe([this]{
        updateLocation();
    });

    setVisible(false);

    WidgetWrapper(this).AddEventFilter([this](QObject*, QEvent* e){
        if(e->type() == QEvent::ShowToParent || e->type() == QEvent::Show) {
            raise();
        }
        return false;
    });
}

ToolTipWidget::~ToolTipWidget()
{

}

void ToolTipWidget::SetContent(QWidget* content, bool deletePrevious)
{
    if(m_content == content) {
        return;
    }
    if(m_content != nullptr) {
        for(qint32 i(0); i < layout()->count(); i++) {
            auto* item = layout()->itemAt(i);
            if(item->widget() == m_content) {
                item->widget()->setParent(nullptr);
                layout()->removeItem(item);
                break;
            }
        }
    }
    if(deletePrevious) {
        delete m_content;
    }
    m_content = content;
    layout()->addWidget(m_content);
}

void ToolTipWidget::SetTarget(const QPoint& target)
{
    if(m_target != target) {
        m_target = target;
        updateLocation();
    }
}

void ToolTipWidget::updateLocation()
{
    QuadTreeF::BoundingRect targetRect(m_target.x(), m_target.y(), 0, 0);
    QuadTreeF::BoundingRect parentRect(parentWidget()->width() / 2, parentWidget()->height() / 2, parentWidget()->width(), parentWidget()->height());

    auto targetLocation = parentRect.locationOfOther(targetRect);
    switch (targetLocation) {
    case QuadTreeF::Location_TopLeft: {
        QRect geometry(m_target.x(), m_target.y(), width(), height());
        geometry.translate(OffsetFromTarget.Native().x(), OffsetFromTarget.Native().y());
        updateGeometry(geometry);
        break;
    }
    default: {
        QRect geometry(m_target.x() - width(), m_target.y(), width(), height());
        geometry.translate(-OffsetFromTarget.Native().x(), OffsetFromTarget.Native().y());
        updateGeometry(geometry);
        break;
    }
    case QuadTreeF::Location_BottomLeft: {
        QRect geometry(m_target.x(), m_target.y() - height(), width(), height());
        geometry.translate(OffsetFromTarget.Native().x(), -OffsetFromTarget.Native().y());
        updateGeometry(geometry);
        break;
    }
    case QuadTreeF::Location_BottomRight: {
        QRect geometry(m_target.x() - width(), m_target.y() - height(), width(), height());
        geometry.translate(-OffsetFromTarget.Native().x(), -OffsetFromTarget.Native().y());
        updateGeometry(geometry);
        break;
    }
    }
}

void ToolTipWidget::updateGeometry(const QRect& irect)
{
    auto validateGeometry = [this](QRect& geometry) {
        QRect pr(0,0, parentWidget()->width(), parentWidget()->height());
        qint32 dx = 0, dy = 0;
        if(pr.bottom() < geometry.bottom()) {
            dy += pr.bottom() - geometry.bottom();
        }
        if(pr.top() > geometry.top()) {
            dy -= geometry.top();
        }
        if(pr.left() > geometry.left()) {
            dx -= geometry.left();
        }
        if(pr.right() < geometry.right()) {
            dx += pr.right() - geometry.right();
        }
        geometry.adjust(dx, dy, dx, dy);
    };

    auto rect = irect;
    validateGeometry(rect);
    m_animation = std::unique_ptr<QPropertyAnimation, std::function<void(QPropertyAnimation*)>>(new QPropertyAnimation(), [](QPropertyAnimation* ptr){
        ptr->deleteLater();
    });
    m_animation->setTargetObject(this);
    m_animation->setPropertyName("geometry");
    m_animation->setEasingCurve(QEasingCurve::InCubic);
    m_animation->setDuration(100);
    m_animation->setStartValue(geometry());
    m_animation->setEndValue(rect);

    m_animation->start();
}


void ToolTipWidget::resizeEvent(QResizeEvent*)
{
    updateLocation();
}
