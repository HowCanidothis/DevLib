#include "widgetsfocusmanager.h"

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
    m_focusedWidget = nullptr;
    m_previousFocusedWidget = nullptr;
}
