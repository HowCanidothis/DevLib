#include "tooltipwidget.h"

#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>

ToolTipWidget::ToolTipWidget(QWidget* parent)
    : Super(parent)
{
    OffsetFromTarget.Subscribe([this]{
        updateLocation();
    });

    setVisible(false);
}

ToolTipWidget::~ToolTipWidget()
{

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
    QuadTreeF::BoundingRect targetRect(m_target.x(), m_target.y(), 1.f, 1.f);
    QuadTreeF::BoundingRect parentRect(parentWidget()->width() / 2, parentWidget()->height() / 2, parentWidget()->width() * 2, parentWidget()->height() * 2);

    auto targetLocation = parentRect.locationOfOther(targetRect);
    switch (targetLocation) {
    case QuadTreeF::Location_TopLeft: {
        QRect geometry(m_target.x(), m_target.y(), width(), height());
        geometry.translate(OffsetFromTarget.Native().x(), OffsetFromTarget.Native().y());
        updateGeometry(geometry);
        break;
    }
    case QuadTreeF::Location_TopRight: {
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
    default:
        break;
    }
}

void ToolTipWidget::updateGeometry(const QRect& rect)
{
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
