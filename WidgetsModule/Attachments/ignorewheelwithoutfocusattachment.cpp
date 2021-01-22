#include "ignorewheelwithoutfocusattachment.h"

#include "WidgetsModule/Utils/widgethelpers.h"

#include <QDoubleSpinBox>
#include <QComboBox>

IgnoreWheelWithoutFocusAttachment::IgnoreWheelWithoutFocusAttachment()
{

}

void IgnoreWheelWithoutFocusAttachment::Attach(QWidget* widget)
{
    widget->installEventFilter(&getInstance());
    widget->setFocusPolicy(Qt::StrongFocus);
}

void IgnoreWheelWithoutFocusAttachment::AttachRecursive(QWidget* widget, const std::function<bool (QWidget* w)>& filter)
{
    WidgetContent::ForeachChildWidget(widget, [filter](QWidget* w){
        if(filter(w)) {
            w->installEventFilter(&getInstance());
            w->setFocusPolicy(Qt::StrongFocus);
        }
    });
}

void IgnoreWheelWithoutFocusAttachment::AttachRecursiveSpinBoxesAndComboBoxes(QWidget* widget)
{
    AttachRecursive(widget, [](QWidget* w) -> bool {
        return qobject_cast<QSpinBox*>(w) != nullptr || qobject_cast<QDoubleSpinBox*>(w) != nullptr || qobject_cast<QComboBox*>(w);
    });
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
