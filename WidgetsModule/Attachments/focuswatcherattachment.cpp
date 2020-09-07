#include "focuswatcherattachment.h"

FocusManager::FocusManager()
    : FocusedWidget(nullptr)
    , m_previousFocusedWidget(nullptr)
{
}

void FocusManager::destroyed(QWidget* target)
{
    m_previousFocusedWidget = target == m_previousFocusedWidget ? nullptr : m_previousFocusedWidget;
    FocusedWidget = target == FocusedWidget ? nullptr : FocusedWidget.Native();
}

void FocusManager::focusGot(QWidget* target)
{
    if(FocusedWidget != target) {
        m_previousFocusedWidget = FocusedWidget;
        FocusedWidget = target;
    }
}

FocusManager& FocusManager::GetInstance()
{
    static FocusManager result;
    return result;
}

FocusWatcherAttachment::FocusWatcherAttachment(QWidget* target)
    : QObject(nullptr)
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

bool FocusWatcherAttachment::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::FocusIn: FocusManager::GetInstance().focusGot(m_target); break;
    case QEvent::Destroy:
        if(m_target == watched) {
            FocusManager::GetInstance().destroyed(m_target);
            deleteLater();
        }
        break;
    default: break;
    }
    return false;
}
