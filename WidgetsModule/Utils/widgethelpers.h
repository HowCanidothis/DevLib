#ifndef WIDGETHELPERS_H
#define WIDGETHELPERS_H

#include <PropertiesModule/internal.hpp>

struct WidgetsLocalPropertyColorWrapperColorMap
{
    qint32 Role;
    LocalPropertyColor* Color;
};
Q_DECLARE_TYPEINFO(WidgetsLocalPropertyColorWrapperColorMap, Q_PRIMITIVE_TYPE);

template<class T>
class WidgetsLocalPropertyDecimalDisplay : public LocalPropertyLimitedDecimal<T>
{
    using Super = LocalPropertyLimitedDecimal<T>;
public:
    WidgetsLocalPropertyDecimalDisplay(const T& value = 0, const T& min = (std::numeric_limits<T>::lowest)(), const T& max = (std::numeric_limits<T>::max)())
        : Super(value, min, max)
    {
        auto update = [this]{
            DisplayValue.SetMinMax(*this, *this);
        };
        this->OnChange.Connect(this, update);
        update();
    }

    WidgetsLocalPropertyDecimalDisplay& operator-=(const T& value) { SetValue(Super::Native() - value); return *this; }
    WidgetsLocalPropertyDecimalDisplay& operator+=(const T& value) { SetValue(Super::Native() + value); return *this; }
    WidgetsLocalPropertyDecimalDisplay& operator=(const T& value) { SetValue(value); return *this; }

    LocalPropertyLimitedDecimal<T> DisplayValue;
};

class WidgetsLocalPropertyColorWrapper : public QObject
{
public:
    WidgetsLocalPropertyColorWrapper(QWidget* widget, const Stack<WidgetsLocalPropertyColorWrapperColorMap>& colorMap);

private:
    void polish();
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    Stack<WidgetsLocalPropertyColorWrapperColorMap> m_properties;
    QWidget* m_widget;
    DelayedCallObject m_updateLater;
    DispatcherConnectionsSafe m_connections;
};

class WidgetsLocalPropertyVisibilityWrapper : public QObject
{
public:
    WidgetsLocalPropertyVisibilityWrapper(QWidget* widget);

    LocalPropertyBool Visible;

private:
    QWidget* m_widget;
    DispatcherConnectionsSafe m_connections;
};

class WidgetsObserver : public QObject
{
    WidgetsObserver();
public:
    static WidgetsObserver& GetInstance();

    CommonDispatcher<QObject*> OnAdded;

private:
    bool eventFilter(QObject* watched, QEvent* e) override;
};

struct WidgetAppearance
{
    static DispatcherConnection ConnectWidgetsByVisibility(WidgetsLocalPropertyVisibilityWrapper* base, WidgetsLocalPropertyVisibilityWrapper* child);
    static void SetVisibleAnimated(QWidget* widget, bool visible);
    static void ShowAnimated(QWidget* widget);
    static void HideAnimated(QWidget* widget);
};

struct WidgetContent
{
    static void ForeachChildWidget(QWidget* target, const std::function<void (QWidget*)>& handler);
    static void CopySelectedTableContentsToClipboard(class QTableView* tableView);
};

#endif // WIDGETHELPERS_H
