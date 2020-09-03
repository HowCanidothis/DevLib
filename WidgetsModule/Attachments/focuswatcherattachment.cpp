#include "focuswatcherattachment.h"

FocusManager& FocusManager::GetInstance()
{
    static FocusManager result;
    return result;
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

void FocusWatcherAttachment::Attach(QWidget* widget)
{
    new FocusWatcherAttachment(widget);
}


bool FocusWatcherAttachment::eventFilter(QObject*, QEvent* event)
{
    bool sendEvent = true;
    switch (event->type()) {
    case QEvent::FocusIn: m_hasFocus = true; break;
    case QEvent::FocusOut: m_hasFocus = m_target->contentsRect().contains(m_target->mapFromGlobal(QCursor::pos())); break;
    default: sendEvent = false; break;
    }
    if(sendEvent) {
        ThreadsBase::DoMain([this]{
            FocusManager::GetInstance().OnFocusChanged(m_target, m_hasFocus);
        });
    }

    return false;
}
