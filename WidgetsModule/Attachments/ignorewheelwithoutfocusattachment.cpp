#include "ignorewheelwithoutfocusattachment.h"

IgnoreWheelWithoutFocusAttachment::IgnoreWheelWithoutFocusAttachment()
{

}

void IgnoreWheelWithoutFocusAttachment::Attach(QWidget* widget)
{
    widget->installEventFilter(&getInstance());
    widget->setFocusPolicy(Qt::ClickFocus);
}

void IgnoreWheelWithoutFocusAttachment::AttachRecursive(QWidget* widget)
{
    auto childWidgets = widget->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        if(childWidget != nullptr) {
            childWidget->installEventFilter(&getInstance());
            childWidget->setFocusPolicy(Qt::ClickFocus);
        }
    }
}

bool IgnoreWheelWithoutFocusAttachment::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Wheel) {
        auto* widget = qobject_cast<QWidget*>(watched);
        if(widget != nullptr && !widget->hasFocus()) {
            event->ignore();
            return true;
        }
    }
    return false;
}

IgnoreWheelWithoutFocusAttachment& IgnoreWheelWithoutFocusAttachment::getInstance()
{
    static IgnoreWheelWithoutFocusAttachment result;
    return result;
}
