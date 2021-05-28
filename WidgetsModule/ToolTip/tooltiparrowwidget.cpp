#include "tooltiparrowwidget.h"

#include <QPainter>

#include <SharedModule/internal.hpp>

ToolTipArrowWidget::ToolTipArrowWidget(QWidget* targetWidget)
    : Super(targetWidget->parentWidget())
    , Size(10)
    , m_targetWidget(targetWidget)
{
    Size.Subscribe([this]{
        updateLocation();
    });

    targetWidget->installEventFilter(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    setVisible(false);
}

void ToolTipArrowWidget::SetTarget(const QPoint& target, bool update)
{
    m_target = target;
    if(update) {
        updateLocation();
    }
}


void ToolTipArrowWidget::paintEvent(QPaintEvent* )
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(palette().background().color());
    painter.setBrush(palette().background().color());
    painter.drawPolygon(m_polygon);
}

void ToolTipArrowWidget::updateLocation()
{
    QuadTreeF::BoundingRect targetRect(m_target.x(), m_target.y(), m_targetWidget->width() - Size * 2, m_targetWidget->height() - Size * 2);
    QuadTreeF::BoundingRect targetWidgetRect(m_targetWidget->x() + m_targetWidget->width() / 2, m_targetWidget->y() + m_targetWidget->height() / 2, m_targetWidget->width(), m_targetWidget->height());

    m_polygon.clear();

    QRect geometry;

    auto targetLocation = targetWidgetRect.locationOfOther(targetRect);
    switch (targetLocation) {
    case QuadTreeF::Location_TopLeft: {
        geometry = QRect(m_target.x(), m_target.y(), qAbs(targetWidgetRect.left() - m_target.x()) + 1, qAbs(targetWidgetRect.top() - m_target.y()) + 1);
        if(geometry.width() < geometry.height()) {
            geometry.adjust(0,0, Size, 0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { w - Size, h }, {0,0}, {w, h}  });

        } else {
            geometry.adjust(0,0, 0, Size);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { w, h }, {0,0}, {w, h - Size}  });
        }
        break;
    }
    case QuadTreeF::Location_TopRight: {
        geometry = QRect(targetWidgetRect.right(), m_target.y(), qAbs(m_target.x() - targetWidgetRect.right()), qAbs(targetWidgetRect.top() - m_target.y()) + 1);
        if(geometry.width() < geometry.height()) {
            geometry.adjust(-Size,0, 0, 0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ {0,h}, { w, 0 }, {Size, h}  });

        } else {
            geometry.adjust(0,0, 0, Size);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { w, 0 }, {0,h}, {0, h - Size}  });
        }
        break;
    }
    case QuadTreeF::Location_BottomLeft: {
        geometry = QRect(m_target.x(), targetWidgetRect.bottom(), qAbs(targetWidgetRect.left() - m_target.x()) + 1, qAbs(m_target.y() - targetWidgetRect.bottom()));
        if(geometry.width() < geometry.height()) {
            geometry.adjust(0,0, Size, 0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ {0,h}, { w - Size, 0 }, {w, 0}  });

        } else {
            geometry.adjust(0,-Size, 0,0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { 0, h }, {w,0}, {w, Size}  });
        }
        break;
    }
    case QuadTreeF::Location_BottomRight: {
        geometry = QRect(targetWidgetRect.right(), targetWidgetRect.bottom(), qAbs(m_target.x() - targetWidgetRect.right()), qAbs(m_target.y() - targetWidgetRect.bottom()));
        if(geometry.width() < geometry.height()) {
            geometry.adjust(-Size,0, 0, 0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ {0,0}, { Size, 0 }, {w, h}  });

        } else {
            geometry.adjust(0,-Size, 0,0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { 0, 0 }, {w,h}, {0, Size}  });
        }
        break;
    }
    case QuadTreeF::Location_MiddleTop: {
        geometry = QRect(m_target.x() - Size / 2, m_target.y(), Size, targetWidgetRect.top() - m_target.y() + 1);
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { w / 2, 0 }, {w,h}, {0, h}  });
        break;
    }
    case QuadTreeF::Location_MiddleRight: {
        geometry = QRect(targetWidgetRect.right(), m_target.y() - Size / 2, m_target.x() - targetWidgetRect.right(), Size);
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { 0, 0 }, {w,h / 2}, {0, h}  });
        break;
    }
    case QuadTreeF::Location_MiddleLeft: {
        geometry = QRect(m_target.x(), m_target.y() - Size / 2, targetWidgetRect.left() - m_target.x() + 1, Size);
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { 0, h/2 }, {w,0}, {w, h}  });
        break;
    }
    case QuadTreeF::Location_MiddleBottom: {
        geometry = QRect(m_target.x() - Size / 2, targetWidgetRect.bottom(), Size, m_target.y() - targetWidgetRect.bottom());
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { 0, 0 }, {w,0}, {w/2, h}  });
        break;
    }
    default:
        break;
    }

    auto visible = parentWidget()->rect().contains(geometry) && m_targetWidget->isVisible();
    if(visible) {
        setGeometry(geometry);
    }
    setVisible(visible);
}


bool ToolTipArrowWidget::eventFilter(QObject*, QEvent* event)
{
    switch(event->type()) {
    case QEvent::Move:
    case QEvent::Resize:
        updateLocation();
        break;
    case QEvent::Show:
        show();
        break;
    case QEvent::Hide:
        hide();
        break;
    default:
        break;
    }

    return false;
}
