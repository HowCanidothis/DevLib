#ifndef WIDGETSGLOBALTABLEACTIONSSCOPE_H
#define WIDGETSGLOBALTABLEACTIONSSCOPE_H

#include "WidgetsModule/widgetsdeclarations.h"
#include "actionsscopebase.h"

class WidgetsGlobalTableActionsScopeHandler
{
public:
    WidgetsGlobalTableActionsScopeHandler() {}
    WidgetsGlobalTableActionsScopeHandler(QAction* uiAction, const FAction& action);

    const WidgetsGlobalTableActionsScopeHandler& SetVisible(bool visible) const;

    LocalPropertyBool& Visibility() const;
    LocalPropertyBool& Enablity() const;
    TranslatedString& Text() const;
    const WidgetsGlobalTableActionsScopeHandler& SetAction(const FAction& action) const;

    FAction GetAction() const;
    const WidgetsGlobalTableActionsScopeHandler& SetShortcut(const QKeySequence& shortcut) const;
    void Trigger();

    bool IsValid() const { return m_data != nullptr; }

private:
    friend class WidgetsGlobalTableActionsScope;
    SharedPointer<struct WidgetsGlobalTableActionsScopeHandlerData> m_data;
};

struct WidgetsGlobalTableActionsScopeHandlers
{
    QHash<QAction*, WidgetsGlobalTableActionsScopeHandler> Handlers;
    LocalPropertyBool IsReadOnly;

    WidgetsGlobalTableActionsScopeHandlers()
        : IsReadOnly(true)
    {}

    WidgetsGlobalTableActionsScopeHandlers& ShowAll();
    WidgetsGlobalTableActionsScopeHandlers& SetActionsVisible(const QVector<Latin1Name>& ids, bool visible);

    WidgetsGlobalTableActionsScopeHandler AddHandler(const FTranslationHandler& handler, const QKeySequence& sequence, QAction* action, const FAction& actionHandler);
    WidgetsGlobalTableActionsScopeHandler AddHandler(const FTranslationHandler& handler, const QKeySequence& sequence, const class WidgetsGlobalTableActionId& actionId, const FAction& actionHandler);
    WidgetsGlobalTableActionsScopeHandler FindHandler(const Latin1Name& id);
};

using FWidgetsGlobalTableActionsScopeHandlersConnector = std::function<void (const WidgetsGlobalTableActionsScopeHandler& handler)>;
Q_DECLARE_METATYPE(SharedPointer<WidgetsGlobalTableActionsScopeHandlers>)
Q_DECLARE_METATYPE(SharedPointer<FWidgetsGlobalTableActionsScopeHandlersConnector>)

class WidgetsGlobalTableActionsScope : public ActionsScopeBase, public Singletone<WidgetsGlobalTableActionsScope>
{
    using Super = ActionsScopeBase;
    using FConnector = FWidgetsGlobalTableActionsScopeHandlersConnector;
public:
    enum EnableIfMode
    {
        EIM_Default,
        EIM_TableHasSelection = 0x1,
        EIM_TableSelectionOnlyOne = 0x2,
        EIM_TableIsEditable = 0x4,

        EIM_TableHasSelectionAndIsEditable = EIM_TableHasSelection | EIM_TableIsEditable,
        EIM_TableSelectionOnlyOneAndIsEditable = EIM_TableSelectionOnlyOne | EIM_TableIsEditable
    };

    WidgetsGlobalTableActionsScope();

    static WidgetsGlobalTableActionsScopeHandlersPtr EditHandlers(class QTableView* table);
    static WidgetsGlobalTableActionsScopeHandlersPtr AddDefaultHandlers(QTableView* table, bool assert = true);

    static FAction CreateDefaultDeleteHandler(QTableView* table);
    static FAction CreateDefaultPasteHandler(QTableView* table);
    static QList<QString> ClipboardRows();

private:
    friend class WidgetsGlobalTableActionId;
    QAction* registerAction(const Latin1Name& id, EnableIfMode mode = EIM_Default, const Name& icon = Name());
    virtual void CreateActions(){}

private:
    LocalPropertySharedPtr<WidgetsGlobalTableActionsScopeHandlers> m_currentHandlers;
    DispatcherConnectionsSafe m_currentHandlersConnections;
};

class WidgetsGlobalTableActionId : public Latin1Name
{
    using Super = Latin1Name;
public:
    WidgetsGlobalTableActionId(const char* id, WidgetsGlobalTableActionsScope::EnableIfMode mode, const Name& icon = Name());

private:
    struct ActionInfo {
        Latin1Name Id;
        WidgetsGlobalTableActionsScope::EnableIfMode Mode;
        Name Icon;
    };
    friend class WidgetsGlobalTableActionsScope;
    static QVector<ActionInfo> m_delayedRegistration;
};

namespace ActionIcons {
DECLARE_GLOBAL_NAME(NoIcon)
DECLARE_GLOBAL_NAME(Open)
DECLARE_GLOBAL_NAME(Clone)
DECLARE_GLOBAL_NAME(Copy)
DECLARE_GLOBAL_NAME(Cut)
DECLARE_GLOBAL_NAME(Paste)
DECLARE_GLOBAL_NAME(Delete)
DECLARE_GLOBAL_NAME(Insert)
DECLARE_GLOBAL_NAME(Download)
DECLARE_GLOBAL_NAME(Settings)
DECLARE_GLOBAL_NAME(ResolveConflictServer)
DECLARE_GLOBAL_NAME(ResolveConflictClient)
DECLARE_GLOBAL_NAME(Burger)
DECLARE_GLOBAL_NAME(DropDown)
}

#define DECLARE_GLOBAL_ACTION(ActionId) \
    const extern WidgetsGlobalTableActionId ActionId;

#define IMPLEMENT_GLOBAL_ACTION(ActionId, mode, icon) \
    const WidgetsGlobalTableActionId ActionId(#ActionId, mode, icon);

DECLARE_GLOBAL_ACTION(GlobalActionCopyId);
DECLARE_GLOBAL_ACTION(GlobalActionCopyWithHeadersId);
DECLARE_GLOBAL_ACTION(GlobalActionCutId);
DECLARE_GLOBAL_ACTION(GlobalActionPasteId);
DECLARE_GLOBAL_ACTION(GlobalActionDeleteId);
DECLARE_GLOBAL_ACTION(GlobalActionInsertId);
DECLARE_GLOBAL_ACTION(GlobalActionImportId);
DECLARE_GLOBAL_ACTION(GlobalActionDebugJSONId);
DECLARE_GLOBAL_ACTION(GlobalActionDebugSelectId);

#endif // WIDGETSGLOBALTABLEACTIONSSCOPE_H
