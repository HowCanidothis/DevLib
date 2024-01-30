#include "windowresizeattachment.h"

#include <QHoverEvent>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QWindow>
#include <QWidget>

WindowResizeAttachment::WindowResizeAttachment()
    : m_borderWidth(8)
    , m_draggingLocation(Location_Default)
    , m_previousMoveLocation(Location_Default)
{
}

void WindowResizeAttachment::Attach(QWidget* widget)
{
    widget->installEventFilter(&instance());
}

WindowResizeAttachment::Location WindowResizeAttachment::findLocation(QWindow* window, const QPoint& pos)
{
    if (window->visibility() != QWindow::Windowed) {
        return Location_Default;
    }
    auto rect = window->frameGeometry();

    return FindLocation(rect, pos, m_borderWidth);
}

WindowResizeAttachment::Location WindowResizeAttachment::FindLocation(const QRectF& rect, const QPointF& pos, qint32 border)
{
    qint32 location = Location_Default;
    auto leftDist = abs(pos.x() - rect.left());
    auto rightDist = abs(rect.right() - pos.x());
    auto leftPrior = leftDist <= rightDist;
    auto topDist = abs(pos.y() - rect.top());
    auto bottomDist = abs(rect.bottom() - pos.y());
    auto topPrior = topDist <= bottomDist;
    if(leftPrior) {
        location |= leftDist <= border ? Location_Left : Location_Default;
    } else {
        location |= rightDist <= border ? Location_Right : Location_Default;
    }
    if(topPrior) {
        location |= topDist <= border ? Location_Top : Location_Default;
    } else {
        location |= bottomDist <= border ? Location_Bottom : Location_Default;
    }
    return (Location)location;
}



void WindowResizeAttachment::resize(QWindow* window, const QPoint& screenPos, Location location)
{
    auto rect = window->geometry();
    auto rectSource = rect;

    auto funcUpdateTop = [&rect, &rectSource, window, &screenPos]{
        auto deltaY = rect.top() - screenPos.y();
        rect.setY(rectSource.top() - deltaY);
        if (window->minimumHeight() > rect.height()) {
            rect.setY(rectSource.bottom() - window->minimumHeight());
        }
    };
    auto funcUpdateBottom = [&rect, &screenPos, window]{
        auto deltaY = rect.bottom() - screenPos.y();
        auto newHeight = rect.height() - deltaY;
        if(newHeight >= window->minimumHeight()) {
            rect.setHeight(rect.height() - deltaY);
        } else {
            rect.setHeight(window->minimumHeight());
        }
    };
    auto funcUpdateLeft = [&rect, &rectSource, window, &screenPos]{
        auto deltaX = rect.left() - screenPos.x();
        rect.setX(rect.left() - deltaX);
        if (window->minimumWidth() > rect.width()) {
            rect.setX(rectSource.right() - window->minimumWidth());
        }
    };
    auto funcUpdateRight = [&rect, &screenPos, window]{
        auto deltaX = rect.right() - screenPos.x();
        auto newWidth = rect.width() - deltaX;
        if(newWidth >= window->minimumWidth()) {
            rect.setWidth(rect.width() - deltaX);
        } else {
            rect.setWidth(window->minimumWidth());
        }
    };

    switch (location) {
    case Location_Top: funcUpdateTop(); break;
    case Location_Bottom: funcUpdateBottom(); break;
    case Location_Left: funcUpdateLeft(); break;
    case Location_Right: funcUpdateRight(); break;
    case Location_TopLeft:
        funcUpdateTop();
        funcUpdateLeft();
        break;
    case Location_BottomLeft:
        funcUpdateBottom();
        funcUpdateLeft();
        break;
    case Location_BottomRight:
        funcUpdateBottom();
        funcUpdateRight();
        break;
    case Location_TopRight:
        funcUpdateTop();
        funcUpdateRight();
        break;
    default:
        break;
    }
    window->setGeometry(rect);
}

bool WindowResizeAttachment::eventFilter(QObject* watched, QEvent* event)
{
    auto window = qobject_cast<QWindow*>(watched);
    if(window == nullptr) { // It is supported to use a widget instead of window, window will be found automatically, when it created
        window = qobject_cast<QWidget*>(watched)->windowHandle();
        if(window != nullptr) {
            window->setMinimumSize(qobject_cast<QWidget*>(watched)->minimumSize());
            window->installEventFilter(this);
            watched->removeEventFilter(this);
        }
        return false;
    }

    switch(event->type()) {
    case QEvent::MouseMove: {
        auto mouseMoveEvent = reinterpret_cast<QMouseEvent*>(event);
        if(m_draggingLocation != Location_Default) {
            resize(window, mouseMoveEvent->globalPos(), m_draggingLocation);
            return true;
        } else if(mouseMoveEvent->buttons() == Qt::NoButton){
            auto location = findLocation(window, mouseMoveEvent->globalPos());
            if(location != Location_Default) {
                QHoverEvent leaveEvent(QHoverEvent::Leave, mouseMoveEvent->pos(), mouseMoveEvent->pos(), mouseMoveEvent->modifiers());
                qApp->sendEvent(window, &leaveEvent);
                InstallCursorFromLocation(window, location);
                m_previousMoveLocation = location;
                return true;
            } else if(m_previousMoveLocation != Location_Default){
                QHoverEvent hoverEvent(QHoverEvent::Enter, mouseMoveEvent->pos(), mouseMoveEvent->pos(), mouseMoveEvent->modifiers());
                qApp->sendEvent(window, &hoverEvent);
            }
            m_previousMoveLocation = location;
        }
        break;
    }
    case QEvent::CursorChange:
        if(m_draggingLocation != Location_Default) {
            return true;
        }
        return false;
    case QEvent::MouseButtonPress: {
        auto mousePressEvent = reinterpret_cast<QMouseEvent*>(event);
        auto location = findLocation(window, mousePressEvent->globalPos());
        if (mousePressEvent->button() & Qt::LeftButton && location != Location_Default) {
            m_draggingLocation = location;
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        if(m_draggingLocation != Location_Default) {
            m_draggingLocation = Location_Default;
            return true;
        }
        break;
    }
    case QEvent::MouseButtonDblClick: {
        auto mousePressEvent = reinterpret_cast<QMouseEvent*>(event);
        return findLocation(window, mousePressEvent->globalPos()) != Location_Default;
    }
    default: break;
    }
    
    return false;
}

WindowResizeAttachment& WindowResizeAttachment::instance()
{
    static WindowResizeAttachment result;
    return result;
}
