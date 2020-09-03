#include "focuswatcherattachment.h"

FocusManager& FocusManager::GetInstance()
{
    static FocusManager result;
    return result;
}

void FocusManager::ResetFocus()
{
    widgetFocused(nullptr);
}

FocusWatcherAttachment::FocusWatcherAttachment(QWidget* target)
    : QObject(target)
    , m_target(target)
{
    auto childWidgets = target->findChildren<QWidget*>();
    target->installEventFilter(this);
    for(auto* childWidget : childWidgets) {
        if(childWidget != nullptr) {
            childWidget->installEventFilter(this);
        }
    }    
}

void FocusManager::widgetDestroyed(QWidget* widget)
{
    if(m_previousFocusedWidget == widget) {
        m_previousFocusedWidget = nullptr;
    }
    if(FocusedWidget == widget) {
        FocusedWidget = nullptr;
    }
}

void FocusManager::widgetFocused(QWidget* widget)
{
    if(FocusedWidget != widget) {
        m_previousFocusedWidget = FocusedWidget.Native();
        FocusedWidget = widget;
    }
}

void FocusWatcherAttachment::Attach(QWidget* widget)
{
    new FocusWatcherAttachment(widget);
}


bool FocusWatcherAttachment::eventFilter(QObject*, QEvent* event)
{
    switch (event->type()) {
    case QEvent::FocusIn: FocusManager::GetInstance().widgetFocused(m_target); break;
    case QEvent::Destroy: FocusManager::GetInstance().widgetDestroyed(m_target); break;
    default: break;
    }

    return false;
}
