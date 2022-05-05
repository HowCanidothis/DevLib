#ifndef WIDGETSFOCUSMANAGER_H
#define WIDGETSFOCUSMANAGER_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

class FocusManager // TODO. Maybe not static
{
    friend class WidgetWrapper;
    FocusManager();

    void destroyed(QWidget* target);

public:
    static FocusManager& GetInstance();

    void ResetFocus();
    void SetFocusWidget(QWidget* widget);
    QWidget* GetPreviousFocusedWidget() const { return m_previousFocusedWidget; }
    const LocalProperty<QWidget*>& FocusedWidget() const { return m_focusedWidget; }

private:
    QWidget* m_previousFocusedWidget;
    LocalProperty<QWidget*> m_focusedWidget;
};

#endif // WIDGETSFOCUSMANAGER_H
