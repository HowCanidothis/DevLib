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

#define DECLARE_WIDGET_WRAPPER_FUNCTIONS(WrapperType, type) \
    const WrapperType& Make(const std::function<void (const WrapperType&)>& handler) const { return make<WrapperType>(handler); } \
    type* GetWidget() const { return reinterpret_cast<type*>(m_object); } \
    type* operator->() const { return reinterpret_cast<type*>(m_object); } \
    operator type*() const { return reinterpret_cast<type*>(m_object); } \
    using expected_type = type;

class ObjectWrapper
{
public:
    ObjectWrapper(QObject* object)
        : m_object(object)
    {}

    template<class T> const T& Cast() const { Q_ASSERT(qobject_cast<typename T::expected_type*>(m_object)); return *((const T*)this); }

    template<class T>
    SharedPointer<T> Injected(const char* propertyName, const std::function<T* ()>& creator = nullptr) const
    {
        auto value = m_object->property(propertyName).value<SharedPointer<T>>();
        if(value == nullptr) {
            value = creator != nullptr ? creator() : new T();
            m_object->setProperty(propertyName, QVariant::fromValue(value));
        }
        return value;
    }

    SharedPointer<WidgetWrapperInjectedCommutatorData> InjectedCommutator(const char* propertyName, const std::function<void (QObject* w)>& handler = nullptr) const
    {
        return Injected<WidgetWrapperInjectedCommutatorData>(propertyName, [&]() -> WidgetWrapperInjectedCommutatorData* {
            auto* result = new WidgetWrapperInjectedCommutatorData();
            auto* widget = m_object;
            result->Commutator.Connect(CONNECTION_DEBUG_LOCATION, [handler, widget]{
                 handler(widget);
            });
            return result;
        });
    }

    template<class Property, typename ... Args>
    SharedPointer<Property> GetOrCreateProperty(const char* propName, const std::function<void (QObject*, const Property&)>& handler, Args... args) const
    {
        return Injected<Property>(propName, [&]() -> Property* {
            auto* property = new Property(args...);
            auto* widget = m_object;
            property->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget, handler, property]{ handler(widget, *property); });
            property->SetSetterHandler(ThreadHandlerMain);
            return property;
        });
    }

protected:
    template<class T>
    const T& make(const std::function<void (const T&)>& handler) const
    {
        auto* tThis = reinterpret_cast<const T*>(this);
        handler(*tThis);
        return *tThis;
    }

protected:
    QObject* m_object;
};

class WidgetWrapper : public ObjectWrapper
{
    using Super = ObjectWrapper;
public:
    WidgetWrapper(QWidget* widget);

    template<class T>
    T* InjectedWidget(const char* propertyName, const std::function<T* (QWidget* parent)>& creator = nullptr) const
    {
        auto* value = (T*)GetWidget()->property(propertyName).value<size_t>();
        if(value == nullptr) {
            value = creator != nullptr ? creator(GetWidget()) : new T(GetWidget());
            GetWidget()->setProperty(propertyName, (size_t)value);
        }
        return value;
    }

    DispatcherConnection ConnectEnablityFrom(const char* conInfo, QWidget* widget) const;
    DispatcherConnection ConnectVisibilityFrom(const char* conInfo, QWidget* widget) const;

    DispatcherConnection ConnectEnablityTo(const char* conInfo, QWidget* widget) const;
    DispatcherConnection ConnectVisibilityTo(const char* conInfo, QWidget* widget) const;

    DispatcherConnections CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const;
    DispatcherConnections CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const;
    template<typename ... Dispatchers>
    DispatcherConnections CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        return createRule<WidgetWrapper>(debugLocation, QOverload<>::of(&WidgetWrapper::WidgetVisibility), handler, additionalWidgets, dispatchers...);
    }
    template<typename ... Dispatchers>
    DispatcherConnections CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        return createRule<WidgetWrapper>(debugLocation, &WidgetWrapper::WidgetEnablity, handler, additionalWidgets, dispatchers...);
    }

    void ActivateWindow(int mode, qint32 delay = 1000) const;
    void Highlight(qint32 unhightlightIn = 0) const;
    void Lowlight() const;
    void SetVisibleAnimated(bool visible, int duration = 1000, double opacity = 0.8) const;
    void ShowAnimated(int duration = 1000, double opacity = 0.8) const;
    void HideAnimated(int duration = 1000) const;

    class WidgetsLocationAttachment* LocateToParent(const DescWidgetsLocationAttachmentParams& params) const;
    WidgetsLocationAttachment* Location() const;


    const WidgetWrapper& AddModalProgressBar() const;
    const WidgetWrapper& AddToFocusManager(const QVector<QWidget*>& additionalWidgets) const;
    const WidgetWrapper& AddEventFilter(const std::function<bool (QObject*, QEvent*)>& filter) const;
    const WidgetWrapper& AddDisconnectableEventFilter(const std::function<bool (QObject*, QEvent*)>& filter) const;
    const WidgetWrapper& CreateCustomContextMenu(const std::function<void (QMenu*)>& creatorHandler, bool preventFromClosing = false) const;

    const WidgetWrapper& BlockWheel() const;
    const WidgetWrapper& FixUp() const;
    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetWrapper, QWidget)
    const WidgetWrapper& SetPalette(const QHash<qint32, LocalPropertyColor*>& palette) const;

    DispatcherConnectionsSafe& WidgetConnections() const;
    LocalPropertyBool& WidgetHighlighted() const;
    LocalPropertyBool& WidgetVisibility() const { return WidgetVisibility(false); }
    LocalPropertyBool& WidgetVisibility(bool animated) const;
    LocalPropertyBool& WidgetEnablity() const;
    LocalPropertyBool& WidgetCollapsing(Qt::Orientation orientation, qint32 initialWidth) const;
    TranslatedStringPtr WidgetToolTip() const;

    bool HasParent(QWidget* parent) const;
    void ForeachParentWidget(const std::function<bool(const WidgetWrapper&)>& handler) const;
    void ForeachChildWidget(const std::function<void (const WidgetWrapper&)>& handler) const;

protected:
    template<typename T, typename FPropertyGetter, typename ... Dispatchers>
    DispatcherConnections createRule(const char* debugLocation, const FPropertyGetter& propertyGetter, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        DispatcherConnections result;
        auto& targetProperty = (Cast<T>().*propertyGetter)();
        result += targetProperty.ConnectFrom(debugLocation, [handler] { return handler(); }, dispatchers...);
        for(auto* widget : additionalWidgets) {
            result += (WidgetWrapper(widget).Cast<T>().*propertyGetter)().ConnectFrom(debugLocation, targetProperty);
        }
        return result;
    }
};

class WidgetPushButtonWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetPushButtonWrapper(class QPushButton* pushButton);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetPushButtonWrapper, QPushButton)

    LocalPropertyBool& WidgetChecked() const;
    const WidgetPushButtonWrapper& SetIcon(const Name& iconId) const;

    Dispatcher& OnClicked() const;
    TranslatedStringPtr WidgetText() const;
};

class WidgetLineEditWrapper : public WidgetWrapper
{
public:
    WidgetLineEditWrapper(class QLineEdit* lineEdit);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetLineEditWrapper, QLineEdit)
    const WidgetLineEditWrapper& SetDynamicSizeAdjusting() const;

private:
};

class WidgetCheckBoxWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetCheckBoxWrapper(class QCheckBox* target);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetCheckBoxWrapper, QCheckBox)
    LocalPropertyBool& WidgetChecked() const;
    TranslatedStringPtr WidgetText() const;
};

class WidgetComboboxWrapper : public WidgetWrapper
{
public:
    WidgetComboboxWrapper(class QComboBox* combobox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetComboboxWrapper, QComboBox)
    template<class Enum>
    const WidgetComboboxWrapper& SetEnum(const std::function<void (QStringList&)>& specialRule = [](QStringList&){}) const
    {
        auto& connections = *Injected<DispatcherConnectionsSafe>("a_items_connections");
        connections.clear();
        auto *widget = GetWidget();
        TranslatorManager::GetInstance().OnLanguageChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget, specialRule]{
            QSignalBlocker blocker(widget);
            widget->clear();
            const auto& names = TranslatorManager::GetInstance().GetEnumNames<Enum>();
            QStringList list;
            for(const auto& value : adapters::range(names.begin() + (qint32)Enum::First, names.begin() + (qint32)Enum::Last + 1)) {
                list.append(value);
            }
            specialRule(list);
            widget->addItems(list);
        }).MakeSafe(connections);
        return *this;
    }
    const WidgetComboboxWrapper& EnableStandardItems(const QSet<qint32>& indices) const;
    const WidgetComboboxWrapper& DisableStandardItems(const QSet<qint32>& indices) const;
    const WidgetComboboxWrapper& DisconnectModel() const;
    class QCompleter* CreateCompleter(QAbstractItemModel* model, const std::function<void (const QModelIndex& index)>& onActivated, qint32 column = 0) const;

    CommonDispatcher<qint32>& OnActivated() const;
};

class WidgetGroupboxWrapper : public WidgetWrapper
{
public:
    WidgetGroupboxWrapper(class QGroupBox* groupBox);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetGroupboxWrapper, QGroupBox)
    const WidgetGroupboxWrapper& AddCollapsing() const;
    const WidgetGroupboxWrapper& AddCollapsingDispatcher(Dispatcher* updater, class QScrollArea* area = nullptr) const;
};

class LocalPropertyDoubleDisplay : public LocalPropertyDouble
{
    using Super = LocalPropertyDouble;
public:
    LocalPropertyDoubleDisplay(double value = 0, double min = (std::numeric_limits<double>::lowest)(), double max = (std::numeric_limits<double>::max)())
        : Super(value, min, max)
        , Precision(2)
    {
        OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this]{
            DisplayValue.SetMinMax(*this, *this);
        });
    }

    LocalPropertyDoubleDisplay& operator-=(double value) { SetValue(Super::Native() - value); return *this; }
    LocalPropertyDoubleDisplay& operator+=(double value) { SetValue(Super::Native() + value); return *this; }
    LocalPropertyDoubleDisplay& operator=(double value) { SetValue(value); return *this; }

    LocalPropertyDouble DisplayValue;
    LocalPropertyInt Precision;
};

class WidgetLabelWrapper : public WidgetWrapper
{
public:
    WidgetLabelWrapper(class QLabel* label);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetLabelWrapper, QLabel)

    TranslatedStringPtr WidgetText() const;

    CommonDispatcher<const Name&>& OnLinkActivated() const;
};

class WidgetTableViewWrapper : public WidgetWrapper
{
public:
    WidgetTableViewWrapper(class QTableView* tableView);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetTableViewWrapper, QTableView)
    bool CopySelectedTableContentsToClipboard(bool includeHeaders = false) const;
    QList<int> SelectedRowsSorted() const;
    QList<int> SelectedColumnsSorted() const;
    QSet<int> SelectedRowsSet() const;
    QSet<int> SelectedColumnsSet() const;
    void SelectRowsAndScrollToFirst(const QSet<qint32>& rows) const;
    void SelectColumnsAndScrollToFirst(const QSet<qint32>& columns) const;
    class QHeaderView* InitializeHorizontal(const DescTableViewParams& params = DescTableViewParams()) const;
    QHeaderView* InitializeVertical(const DescTableViewParams& params = DescTableViewParams()) const;
    class WidgetsMatchingAttachment* CreateMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns) const;
};

class ActionWrapper : public ObjectWrapper
{
    using Super = ObjectWrapper;
public:
    ActionWrapper(QAction* action);

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(ActionWrapper, QAction);
    const ActionWrapper& SetShortcut(const QKeySequence& keySequence) const;
    const ActionWrapper& SetText(const QString& text) const;

    QAction* GetAction() const { return GetWidget(); }

    LocalPropertyBool& ActionVisibility() const;
    LocalPropertyBool& ActionEnablity() const;
    TranslatedStringPtr ActionText() const;
};

class WidgetSplitterWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetSplitterWrapper(class QSplitter* widget);

    void SetWidgetSize(QWidget* widget, qint32 size) const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetSplitterWrapper, QSplitter);
};

class WidgetDoubleSpinBoxWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetDoubleSpinBoxWrapper(class QDoubleSpinBox* widget);

    template<typename ... Dispatchers>
    DispatcherConnections CreateReadOnlyRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<QWidget*>& additionalWidgets, Dispatchers&... dispatchers) const
    {
        return createRule<WidgetDoubleSpinBoxWrapper>(debugLocation, &WidgetDoubleSpinBoxWrapper::WidgetReadOnly, handler, additionalWidgets, dispatchers...);
    }

    LocalPropertyDouble& WidgetValue() const;
    LocalPropertyBool& WidgetReadOnly() const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetDoubleSpinBoxWrapper, QDoubleSpinBox);
};

class WidgetSpinBoxWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetSpinBoxWrapper(class QSpinBox* widget);

    LocalPropertyInt& WidgetValue() const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetSpinBoxWrapper, QSpinBox);
};

class WidgetDialogWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetDialogWrapper(const Name& id, const std::function<DescCustomDialogParams ()>& paramsCreator);

    template<class T>
    T* GetCustomView() const { return WidgetsDialogsManager::GetInstance().CustomDialogView<T>(GetWidget()); }
    void Show(const DescShowDialogParams& params) const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetDialogWrapper, QDialog)
};

class WidgetHeaderViewWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    WidgetHeaderViewWrapper(class QHeaderView* header);

    LocalPropertyBool& SectionVisibility(qint32 logicalIndex) const;
    const WidgetHeaderViewWrapper& MoveSection(qint32 logicalIndexFrom, qint32 logicalIndexTo) const;

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(WidgetHeaderViewWrapper, QHeaderView);
};

class MenuWrapper : public WidgetWrapper
{
    using Super = WidgetWrapper;
public:
    MenuWrapper(QWidget* widget, const WidgetsGlobalTableActionsScopeHandlersPtr& handlers = nullptr)
        : Super(widget)
        , m_globalActionsHandlers(handlers)
    {}

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(MenuWrapper, QMenu);
    const MenuWrapper& AddGlobalAction(const QString& path) const;
    const MenuWrapper& AddGlobalTableAction(const Latin1Name& id) const;
    ActionWrapper AddAction(const QString& title, const std::function<void ()>& handle) const;
    ActionWrapper AddAction(const QString &title, const std::function<void (QAction*)> &handle) const;

    QMenu* GetMenu() const { return GetWidget(); }

    template<class Property>
    ActionWrapper AddCheckboxAction(const QString& title, Property* value) const
    {
        return AddCheckboxAction(title, *value, [value](bool val){
            *value = val;
        });
    }
    template<class Property>
    ActionWrapper AddColorAction(const QString& title, Property* color) const
    {
        return AddColorAction(title, *color, [color](const QColor& val){
            *color = val;
        });
    }
    template<class Property>
    ActionWrapper AddDoubleAction(const QString& title, Property* value) const
    {
        return AddDoubleAction(title, *value, [value](double val){
            *value = val;
        });
    }

    ActionWrapper AddCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler) const;
    ActionWrapper AddColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler) const;
    ActionWrapper AddDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler) const;
    ActionWrapper AddTableColumnsAction() const;
    ActionWrapper AddSeparator() const;
    class QMenu* AddPreventedFromClosingMenu(const QString& title) const;
    static QMenu* CreatePreventedFromClosingMenu(const QString& title);
    QMenu* AddMenu(const QString& label) const;

private:
    WidgetsGlobalTableActionsScopeHandlersPtr m_globalActionsHandlers;
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

class ViewModelWrapper : public ObjectWrapper
{
    using Super = ObjectWrapper;
public:
    using FIterationHandler = std::function<bool (const QModelIndex& index)>; // if returns true then interrupt iterating
    ViewModelWrapper(QAbstractItemModel* model);

    QAbstractItemModel* GetViewModel() const { return GetWidget(); }

    DECLARE_WIDGET_WRAPPER_FUNCTIONS(ViewModelWrapper, QAbstractItemModel);

    const ViewModelWrapper& ForeachModelIndex(const QModelIndex& parent, const FIterationHandler& function) const;
    const ViewModelWrapper& ForeachModelIndex(const FIterationHandler& function) const { return ForeachModelIndex(QModelIndex(), function); }
    qint32 IndexOf(const FIterationHandler& handler) const;

    Dispatcher& OnReset() const;
};

#endif // WIDGETHELPERS_H
