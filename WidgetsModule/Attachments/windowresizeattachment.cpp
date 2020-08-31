#include "windowresizeattachment.h"

#include <QHoverEvent>
#include <QMouseEvent>

WindowResizeAttachment::WindowResizeAttachment()
    : m_startDrag(false)
    , m_borderWidth(8)
    , m_activeButton(Qt::NoButton)
    , m_cursorSection(Qt::WindowFrameSection::NoSection)
{
//    m_cursorSection.Subscribe([]{
        
//    });
}

void WindowResizeAttachment::Attach(QWidget * widget)
{
    widget->installEventFilter(&instance());
    widget->window()->setMouseTracking(true);
}

void WindowResizeAttachment::AttachRecursive(QWidget* widget)
{
    auto childWidgets = widget->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        if(childWidget != nullptr) {
            childWidget->installEventFilter(&instance());
        }
    }
}

bool WindowResizeAttachment::eventFilter(QObject* watched, QEvent* event)
{
    auto widget = qobject_cast<QWidget*>(watched);
    auto window = widget->window();
    auto funcChangeCursorShape = [this, widget, window](QHoverEvent* event){
        if (m_startDrag) {
            return ;
        }
        if (window->isFullScreen() || window->isMaximized()) {
            if (m_cursorSection != Qt::WindowFrameSection::NoSection) {
                window->unsetCursor();
            }
            return;
        }
        auto pos = widget->mapToGlobal(event->pos());
        auto rect = window->geometry();
        
        
        if(abs(rect.left() - pos.x()) <= m_borderWidth && abs(rect.bottom() - pos.y()) <= m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::BottomLeftSection;
        }
        else if(abs(rect.right() - pos.x()) <= m_borderWidth && abs(rect.bottom() - pos.y()) <= m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::BottomRightSection;
        }
        else if(abs(rect.right() - pos.x()) <= m_borderWidth && abs(rect.top() - pos.y()) <= m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::TopRightSection;
        }
        else if(abs(rect.left() - pos.x()) <= m_borderWidth && abs(rect.top() - pos.y()) <= m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::TopLeftSection;
        }
        else if(abs(pos.x() - rect.left()) <= m_borderWidth && pos.y() <= rect.bottom() - m_borderWidth && pos.y() >= rect.top() + m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::LeftSection;
        }
        else if(abs(pos.x() - rect.right()) <= m_borderWidth && pos.y() <= rect.bottom() - m_borderWidth && pos.y() >= rect.top() + m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::RightSection;
        }
        else if(rect.left() + m_borderWidth < pos.x() && pos.x() < rect.right() + m_borderWidth  && abs(pos.y() - rect.bottom()) <= m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::BottomSection;
        }
        else if(rect.left() + m_borderWidth < pos.x() && pos.x() < rect.right() + m_borderWidth  && abs(pos.y() - rect.top()) <= m_borderWidth)
        {
            m_cursorSection = Qt::WindowFrameSection::TopSection;
        }
        else
        {
            m_cursorSection = Qt::WindowFrameSection::NoSection;
        }
        
        switch (m_cursorSection) {
        case Qt::WindowFrameSection::TopSection:
        case Qt::WindowFrameSection::BottomSection:
            window->setCursor(Qt::SizeVerCursor);
            break;
        case Qt::WindowFrameSection::LeftSection:
        case Qt::WindowFrameSection::RightSection:
            window->setCursor(Qt::SizeHorCursor);
            break;
        case Qt::WindowFrameSection::TopLeftSection:
        case Qt::WindowFrameSection::BottomRightSection:
            window->setCursor(Qt::SizeFDiagCursor);
            break;
        case Qt::WindowFrameSection::TopRightSection:
        case Qt::WindowFrameSection::BottomLeftSection:
            window->setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            window->unsetCursor();
            break;
        }
    };
    
    
    switch(event->type()) {
    case QEvent::MouseMove: {
        auto mouseMoveEvent = reinterpret_cast<QMouseEvent*>(event);
        if (m_startDrag) {
            ///resize
            auto rect = window->geometry();
            auto rectSource = rect;
            
            auto screenPos = mouseMoveEvent->globalPos();
            
            auto funcUpdateTop = [&rect, &rectSource, window, &screenPos]{
                auto deltaY = rect.top() - screenPos.y();
                rect.setY(rectSource.top() - deltaY);
                if (window->minimumHeight() > rect.height()) {
                    rect.setY(rectSource.bottom() - window->minimumHeight());
                }
            };
            auto funcUpdateBottom = [&rect, &screenPos]{
                auto deltaY = rect.bottom() - screenPos.y();
                rect.setHeight(rect.height() - deltaY);
            };
            auto funcUpdateLeft = [&rect, &rectSource, window, &screenPos]{
                auto deltaX = rect.left() - screenPos.x();
                rect.setX(rect.left() - deltaX);
                if (window->minimumWidth() > rect.width()) {
                    rect.setX(rectSource.right() - window->minimumWidth());
                }
            };
            auto funcUpdateRight = [&rect, &screenPos]{
                auto deltaX = rect.right() - screenPos.x();
                rect.setWidth(rect.width() - deltaX);
            };
            
            switch (m_cursorSection) {
            case Qt::WindowFrameSection::TopSection:{
                funcUpdateTop();
                break;
            }
            case Qt::WindowFrameSection::BottomSection:{
                funcUpdateBottom();
                break;
            }
            case Qt::WindowFrameSection::LeftSection:{
                funcUpdateLeft();
                break;
            }
            case Qt::WindowFrameSection::RightSection:{
                funcUpdateRight();
                break;
            }
            case Qt::WindowFrameSection::TopLeftSection:{
                funcUpdateTop();
                funcUpdateLeft();
                break;
            }
            case Qt::WindowFrameSection::BottomLeftSection:{
                funcUpdateBottom();
                funcUpdateLeft();
                break;
            }
            case Qt::WindowFrameSection::BottomRightSection:{
                funcUpdateBottom();
                funcUpdateRight();
                break;
            }
            case Qt::WindowFrameSection::TopRightSection:{
                funcUpdateTop();
                funcUpdateRight();
                break;
            }
            default:
                break;
            }
            window->setGeometry(rect);
            m_startPos = mouseMoveEvent->globalPos();
            return true;
        }
        break;
    }
    case QEvent::HoverMove: {
        auto mouseHoverEvent = reinterpret_cast<QHoverEvent*>(event);
        funcChangeCursorShape(mouseHoverEvent);
        if(m_cursorSection != Qt::WindowFrameSection::NoSection){
            return true;
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        auto mousePressEvent = reinterpret_cast<QMouseEvent*>(event);
        m_activeButton = mousePressEvent->button();
        if (mousePressEvent->button() & Qt::LeftButton && m_cursorSection != Qt::WindowFrameSection::NoSection) {
            m_startDrag = true;
            m_startPos = mousePressEvent->globalPos();
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        m_startDrag = false;
        break;
    }
    default: break;
    }
    
    return false;
}

WindowResizeAttachment & WindowResizeAttachment::instance()
{
    static WindowResizeAttachment result;
    return result;
}
