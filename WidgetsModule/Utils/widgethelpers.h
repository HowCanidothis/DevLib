#ifndef WIDGETHELPERS_H
#define WIDGETHELPERS_H

#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

struct WidgetWrapperInjectedCommutatorData
{
    DispatchersCommutator Commutator;
    DispatcherConnectionsSafe Connections;

    WidgetWrapperInjectedCommutatorData()
        : Commutator(1000)
    {}
};

Q_DECLARE_METATYPE(SharedPointer<WidgetWrapperInjectedCommutatorData>)

#define DECLARE_WIDGET_WRAPPER_FUNCTIONS(WrapperType) \
    WrapperType& Make(const std::function<void (WrapperType&)>& handler) { return make<WrapperType>(handler); }

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

    template<class T>
    SharedPointer<T> Injected(const char* propertyName, const std::function<T* ()>& creator = nullptr) const
    {
        auto value = m_widget->property(propertyName).value<SharedPointer<T>>();
        if(value == nullptr) {
            value = creator != nullptr ? creator() : new T();
            m_widget->setProperty(propertyName, QVariant::fromValue(value));
        }
        return value;
    }

    SharedPointer<WidgetWrapperInjectedCommutatorData> InjectedCommutator(const char* propertyName, const std::function<void (QWidget* w)>& handler = nullptr) const
    {
        return Injected<WidgetWrapperInjectedCommutatorData>(propertyName, [&]() -> WidgetWrapperInjectedCommutatorData* {
            auto* result = new WidgetWrapperInjectedCommutatorData();
            auto* widget = m_widget;
            result->Commutator.Connect(nullptr, [handler, widget]{
                 handler(widget);
            });
            return result;
        });
    }

    void SetVisibleAnimated(bool visible, int duration = 2000, double opacity = 0.8);
    void ShowAnimated(int duration = 2000, double opacity = 0.8);
    void HideAnimated(int duration = 2000);

    WidgetWrapper& AddModalProgressBar();
    WidgetWrapper& AddToFocusManager(const QVector<QWidget*>& additionalWidgets);
    WidgetWrapper& AddEventFilter(const std::function<bool (QObject*, QEvent*)>& filter);

    WidgetWrapper& BlockWheel();
    WidgetWrapper& FixUp();
    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetWrapper)
    WidgetWrapper& SetPalette(const QHash<qint32, LocalPropertyColor*>& palette);
    WidgetWrapper& AttachEventFilter(const std::function<bool (QObject*, QEvent*)>& eventFilter);

    DispatcherConnectionsSafe& WidgetConnections();
    LocalPropertyBool& WidgetVisibility(bool animated = false);
    LocalPropertyBool& WidgetEnablity();

    bool HasParent(QWidget* parent);
    void ForeachParentWidget(const std::function<bool(QWidget*)>& handler);
    void ForeachChildWidget(const std::function<void (QWidget*)>& handler);

    QWidget* operator->() const { return m_widget; }
    operator QWidget*() const { return m_widget; }

private:
    DispatcherConnections createRule(const char* debugLocation, LocalPropertyBool* property, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets,
                                     const FConnector& connector);

#ifdef PROPERTIES_LIB
    template<class Property, typename ... Args>
    SharedPointer<Property> getOrCreateProperty(const char* propName, const std::function<void (QWidget*, const Property&)>& handler, Args... args) const
    {
        return Injected<Property>(propName, [&]() -> Property* {
            auto* property = new Property(args...);
            auto* widget = m_widget;
            property->OnChanged.ConnectAndCall(this, [widget, handler, property]{ handler(widget, *property); });
            property->SetSetterHandler(ThreadHandlerMain);
            return property;
        });
    }
#endif

protected:
    template<class T>
    T& make(const std::function<void (T&)>& handler)
    {
        auto* tThis = reinterpret_cast<T*>(this);
        handler(*tThis);
        return *tThis;
    }

protected:
    QWidget* m_widget;
};

class WidgetLineEditWrapper : public WidgetWrapper
{
public:
    WidgetLineEditWrapper(class QLineEdit* lineEdit);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetLineEditWrapper)
    WidgetLineEditWrapper& SetDynamicSizeAdjusting();

private:
    QLineEdit* lineEdit() const { return reinterpret_cast<QLineEdit*>(m_widget); }
};

class WidgetComboboxWrapper : public WidgetWrapper
{
public:
    WidgetComboboxWrapper(class QComboBox* combobox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetComboboxWrapper)
    WidgetComboboxWrapper& EnableStandardItems(const QSet<qint32>& indices);
    WidgetComboboxWrapper& DisableStandardItems(const QSet<qint32>& indices);
    WidgetComboboxWrapper& DisconnectModel();

private:
    QComboBox* combobox() const { return reinterpret_cast<QComboBox*>(m_widget); }
};

class WidgetGroupboxWrapper : public WidgetWrapper
{
public:
    WidgetGroupboxWrapper(class QGroupBox* groupBox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetGroupboxWrapper)
    WidgetGroupboxWrapper& AddCollapsing();
    WidgetGroupboxWrapper& AddCollapsingDispatcher(Dispatcher* updater);

private:
    QGroupBox* groupBox() const { return reinterpret_cast<QGroupBox*>(m_widget); }
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

class WidgetTableViewWrapper : public WidgetWrapper
{
public:
    WidgetTableViewWrapper(QTableView* tableView);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetTableViewWrapper)
    bool CopySelectedTableContentsToClipboard();
    QList<int> SelectedRowsSorted();
    QList<int> SelectedColumnsSorted();
    QSet<int> SelectedRowsSet();
    QSet<int> SelectedColumnsSet();
    void SelectRowsAndScrollToFirst(const QSet<qint32>& rows);
    void SelectColumnsAndScrollToFirst(const QSet<qint32>& columns);
    class QHeaderView* InitializeHorizontal(const DescColumnsParams& params = DescColumnsParams());
    QHeaderView* InitializeVertical(const DescColumnsParams& params = DescColumnsParams());
    class WidgetsMatchingAttachment* CreateMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns);

private:
    QTableView* tableView() const { return reinterpret_cast<QTableView*>(m_widget); }
};

class WidgetsObserver : public QObject
{
    WidgetsObserver();
public:
    static WidgetsObserver& GetInstance();

    void EnableAutoCollapsibleGroupboxes();

    CommonDispatcher<QWidget*> OnAdded;

private:
    bool eventFilter(QObject* watched, QEvent* e) override;
};

#endif // WIDGETHELPERS_H
