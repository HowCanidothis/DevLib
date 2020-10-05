#ifndef WIDGETHELPERS_H
#define WIDGETHELPERS_H

struct WidgetAppearance
{
    static void SetVisibleAnimated(QWidget* widget, bool visible);
    static void ShowAnimated(QWidget* widget);
    static void HideAnimated(QWidget* widget);
};

struct WidgetContent
{
    static void ForeachChildWidget(QWidget* target, const std::function<void (QWidget*)>& handler);
};

#endif // WIDGETHELPERS_H
