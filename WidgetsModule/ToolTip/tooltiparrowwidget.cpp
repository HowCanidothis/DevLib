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
    painter.setPen(palette().base().color());
    painter.setBrush(palette().base().color());
    painter.drawPolygon(m_polygon);
}

void ToolTipArrowWidget::updateLocation()
{
    QuadTreeF::BoundingRect targetRect(m_target.x(), m_target.y(), m_targetWidget->width() - Size * 2, m_targetWidget->height() - Size * 2);
    QuadTreeF::BoundingRect targetWidgetRect(m_targetWidget->x() + m_targetWidget->width() / 2, m_targetWidget->y() + m_targetWidget->height() / 2, m_targetWidget->width(), m_targetWidget->height());

    m_polygon.clear();

    auto applyBorder = []{ return 1; };
    auto applyTopLeftBorder = []{ return 0; };

    QRect geometry;

    auto targetLocation = targetWidgetRect.locationOfOther(targetRect);
    switch (targetLocation) {
    case QuadTreeF::Location_TopLeft: {
        geometry = QRect(m_target, QPoint(targetWidgetRect.left(), targetWidgetRect.top()));
        if(geometry.width() < geometry.height()) {
            geometry.adjust(0,0, Size, applyTopLeftBorder());
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { w - Size, h }, {0,0}, {w, h}  });

        } else {
            geometry.adjust(0,0, applyTopLeftBorder(), Size);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { w, h }, {0,0}, {w, h - Size}  });
        }
        break;
    }
    case QuadTreeF::Location_TopRight: {
        geometry = QRect(QPoint(targetWidgetRect.right(), m_target.y()), QPoint(m_target.x(), targetWidgetRect.top()));
        if(geometry.width() < geometry.height()) {
            geometry.adjust(-Size,0, 0, applyTopLeftBorder());
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ {0,h}, { w, 0 }, {Size, h}  });

        } else {
            geometry.adjust(applyBorder(),0, 0, Size);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { w, 0 }, {0,h}, {0, h - Size}  });
        }
        break;
    }
    case QuadTreeF::Location_BottomLeft: {
        geometry = QRect(QPoint(m_target.x(), targetWidgetRect.bottom()), QPoint(targetWidgetRect.left(), m_target.y()));
        if(geometry.width() < geometry.height()) {
            geometry.adjust(0,applyBorder(), Size, 0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ {0,h}, { w - Size, 0 }, {w, 0}  });

        } else {
            geometry.adjust(0,-Size, applyTopLeftBorder(),0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { 0, h }, {w,0}, {w, Size}  });
        }
        break;
    }
    case QuadTreeF::Location_BottomRight: {
        geometry = QRect(QPoint(targetWidgetRect.right(), targetWidgetRect.bottom()), m_target);
        if(geometry.width() < geometry.height()) {
            geometry.adjust(-Size,applyBorder(), 0, 0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ {0,0}, { Size, 0 }, {w, h}  });

        } else {
            geometry.adjust(applyBorder(),-Size, 0,0);
            auto w = geometry.width();
            auto h = geometry.height();
            m_polygon = QPolygon({ { 0, 0 }, {w,h}, {0, Size}  });
        }
        break;
    }
    case QuadTreeF::Location_MiddleTop: {
        geometry = QRect(QPoint(m_target.x() - Size / 2, m_target.y()), QPoint(m_target.x() + Size / 2, targetWidgetRect.top() + applyTopLeftBorder()));
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { w / 2, 0 }, {w,h}, {0, h}  });
        break;
    }
    case QuadTreeF::Location_MiddleRight: {
        geometry = QRect(QPoint(targetWidgetRect.right() + applyBorder(), m_target.y() - Size / 2), QPoint(m_target.x(), m_target.y() + Size / 2));
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { 0, 0 }, {w,h / 2}, {0, h}  });
        break;
    }
    case QuadTreeF::Location_MiddleLeft: {
        geometry = QRect(QPoint(m_target.x(), m_target.y() - Size / 2), QPoint(targetWidgetRect.left() + applyTopLeftBorder(), m_target.y() + Size / 2));
        auto w = geometry.width();
        auto h = geometry.height();
        m_polygon = QPolygon({ { 0, h/2 }, {w,0}, {w, h}  });
        break;
    }
    case QuadTreeF::Location_MiddleBottom: {
        geometry = QRect(QPoint(m_target.x() - Size / 2, targetWidgetRect.bottom() + applyBorder()), QPoint(m_target.x() + Size / 2, m_target.y()));
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
        updateLocation();
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
