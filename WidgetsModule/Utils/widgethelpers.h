#ifndef WIDGETHELPERS_H
#define WIDGETHELPERS_H

#include <PropertiesModule/internal.hpp>

class WidgetsAttachment : public QObject
{
    using Super = QObject;
public:
    using FFilter = std::function<bool (QObject*, QEvent*)>;
    WidgetsAttachment(const FFilter& filter, QObject* parent);

    static void Attach(QObject* target, const FFilter& filter);
    static QLineEdit* AttachLineEditAdjuster(QLineEdit* edit);

private:
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    FFilter m_filter;
};

class WidgetWrapper
{
    using FConnector = DispatcherConnection (WidgetWrapper::*)(const char*, QWidget*);
public:
    WidgetWrapper(QWidget* widget);

    DispatcherConnection ConnectVisibility(const char* debugLocation, QWidget* another);
    DispatcherConnection ConnectEnablity(const char* debugLocation, QWidget* another);
    DispatcherConnections CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets)
    {
        return createRule(debugLocation, &WidgetVisibility(), handler, dispatchers, additionalWidgets, &WidgetWrapper::ConnectVisibility);
    }
    DispatcherConnections CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets)
    {
        return createRule(debugLocation, &WidgetEnablity(), handler, dispatchers, additionalWidgets, &WidgetWrapper::ConnectEnablity);
    }

    WidgetWrapper& Make(const std::function<void (ActionWrapper&)>& handler);
    WidgetWrapper& SetPalette(const QHash<qint32, LocalPropertyColor*>& palette);
    WidgetWrapper& AttachEventFilter(const std::function<bool (QObject*, QEvent*)>& eventFilter);

    LocalPropertyBool& WidgetVisibility();
    LocalPropertyBool& WidgetEnablity();

    QWidget* operator->() const { return m_widget; }
    operator QWidget*() const { return m_widget; }

private:
    DispatcherConnections createRule(const char* debugLocation, LocalPropertyBool* property, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets,
                                     const FConnector& connector);

#ifdef PROPERTIES_LIB
    template<class Property, typename ... Args>
    SharedPointer<Property> getOrCreateProperty(const char* propName, const std::function<void (QWidget*, const Property&)>& handler, Args... args) const
    {
        SharedPointer<Property> property = m_widget->property(propName).value<SharedPointer<Property>>();
        if(property == nullptr) {
            auto* action = m_widget;
            property = ::make_shared<Property>(args...);
            m_widget->setProperty(propName, QVariant::fromValue(property));
            auto* pProperty = property.get();
            property->OnChanged.ConnectAndCall(this, [action, handler, pProperty]{ handler(action, *pProperty); });
            property->SetSetterHandler(ThreadHandlerMain);
        }
        return property;
    }
#endif

private:
    QWidget* m_widget;
};

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
        this->OnChanged.Connect(this, update);
        update();
    }

    WidgetsLocalPropertyDecimalDisplay& operator-=(const T& value) { SetValue(Super::Native() - value); return *this; }
    WidgetsLocalPropertyDecimalDisplay& operator+=(const T& value) { SetValue(Super::Native() + value); return *this; }
    WidgetsLocalPropertyDecimalDisplay& operator=(const T& value) { SetValue(value); return *this; }

    LocalPropertyLimitedDecimal<T> DisplayValue;
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
    static void SetVisibleAnimated(QWidget* widget, bool visible, int duration = 2000);
    static void ShowAnimated(QWidget* widget, int duration = 2000);
    static void HideAnimated(QWidget* widget, int duration = 2000);
};

struct WidgetContent
{
    static bool HasParent(QWidget* child, QWidget* parent);
    static void ForeachParentWidget(QWidget* target, const std::function<bool(QWidget*)>& handler);
    static void ForeachChildWidget(QWidget* target, const std::function<void (QWidget*)>& handler);
    static void CopySelectedTableContentsToClipboard(class QTableView* tableView);
    static QList<int> SelectedRowsSorted(QTableView* tableView);
	static QList<int> SelectedColumnsSorted(QTableView* tableView);
	static QSet<int> SelectedRowsSet(QTableView* tableView);
	static QSet<int> SelectedColumnsSet(QTableView* tableView);
    static void SelectRowsAndScrollToFirst(QTableView* table, const QSet<qint32>& rows);
    static void SelectColumnsAndScrollToFirst(QTableView* table, const QSet<qint32>& columns);
    static void ComboboxDisconnectModel(class QComboBox* combobox);
};

#endif // WIDGETHELPERS_H
