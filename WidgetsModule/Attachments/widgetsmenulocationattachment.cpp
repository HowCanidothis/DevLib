#include "widgetsmenulocationattachment.h"

#include <QMenu>
#include <QMenuBar>

void WidgetsMenuLocationAttachment::Attach(QMenu* menu, QuadTreeF::BoundingRect_Location location)
{
    menu->setWindowFlags(menu->windowFlags() | Qt::NoDropShadowWindowHint);
    menu->setProperty("ShowingLocation", location);
    menu->installEventFilter(&getInstance());
}

bool WidgetsMenuLocationAttachment::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Show) {
        auto* menu = reinterpret_cast<QMenu*>(watched);
        QSize parentOffset;
        QPoint menuPos;
        auto* parentWidget = menu->parentWidget();
        if(parentWidget != nullptr) {
            auto* menuBar = qobject_cast<QMenuBar*>(menu->parentWidget());
            if(menuBar != nullptr) {
                auto geometry = menuBar->actionGeometry(menu->menuAction());
                menuPos = parentWidget->mapToGlobal(QPoint(geometry.x(), parentWidget->height()));
                parentOffset = geometry.size();
            } else {
                menuPos = parentWidget->mapToGlobal(QPoint(0, parentWidget->height()));
                parentOffset = menu->parentWidget()->size();
            }
        }

        switch (menu->property("ShowingLocation").toInt()) {
        case QuadTreeF::Location_MiddleTop:
            menu->move(menuPos + QPoint(0, -menu->height() - parentOffset.height()));
            break;
        case QuadTreeF::Location_TopLeft:
            menu->move(menuPos + QPoint(-menu->width(), -menu->height() - parentOffset.height()));
            break;
        case QuadTreeF::Location_MiddleLeft:
            menu->move(menuPos + QPoint(-menu->width(), -parentOffset.height()));
            break;
        case QuadTreeF::Location_BottomLeft:
            menu->move(menuPos + QPoint(-menu->width(), 0));
            break;
        case QuadTreeF::Location_BottomRight:
            menu->move(menuPos + QPoint(parentOffset.width(), 0));
            break;
        case QuadTreeF::Location_MiddleRight:
            menu->move(menuPos + QPoint(parentOffset.width(), -parentOffset.height()));
            break;
        case QuadTreeF::Location_TopRight:
            menu->move(menuPos + QPoint(parentOffset.width(), -parentOffset.height() - menu->height()));
            break;
        case QuadTreeF::Location_Center:
            menu->move(menuPos + QPoint(0, -parentOffset.height()));
            break;
        default: break;
        }

        return false;
    }
    return false;
}

WidgetsMenuLocationAttachment& WidgetsMenuLocationAttachment::getInstance()
{
    static WidgetsMenuLocationAttachment result;
    return result;
}
