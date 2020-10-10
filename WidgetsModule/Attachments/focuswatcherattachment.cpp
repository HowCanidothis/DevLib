#include "focuswatcherattachment.h"

#include "WidgetsModule/Utils/widgethelpers.h"

FocusManager::FocusManager()
    : m_previousFocusedWidget(nullptr)
    , m_focusedWidget(nullptr)
{
}

void FocusManager::destroyed(QWidget* target)
{
    m_previousFocusedWidget = target == m_previousFocusedWidget ? nullptr : m_previousFocusedWidget;
    m_focusedWidget = target == m_focusedWidget ? nullptr : m_focusedWidget.Native();
}

FocusManager& FocusManager::GetInstance()
{
    static FocusManager result;
    return result;
}

void FocusManager::SetFocusWidget(QWidget* target)
{
    if(m_focusedWidget != target) {
        m_previousFocusedWidget = m_focusedWidget;
        m_focusedWidget = target;
    }
}

void FocusManager::ResetFocus()
{
    m_previousFocusedWidget = nullptr;
    m_focusedWidget = nullptr;
}

FocusWatcherAttachment::FocusWatcherAttachment(QWidget* target)
    : QObject(nullptr)
    , m_target(target)
{
    auto childWidgets = target->findChildren<QWidget*>();

    target->installEventFilter(this);
    WidgetContent::ForeachChildWidget(target, [this](QWidget* child){
        child->installEventFilter(this);
    });
}

void FocusWatcherAttachment::Attach(QWidget* widget, const QWidgetList& additionalWidgets)
{
    auto focusWidgetAttachment = new FocusWatcherAttachment(widget);
    for(auto* widget : additionalWidgets) {
        widget->installEventFilter(focusWidgetAttachment);
    }
}

bool FocusWatcherAttachment::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::FocusIn: FocusManager::GetInstance().SetFocusWidget(m_target); break;
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
