#ifndef WIDGETSGLOBALTABLEACTIONSSCOPE_H
#define WIDGETSGLOBALTABLEACTIONSSCOPE_H

#include <ActionsModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsGlobalTableActionsScopeHandler
{
public:
    WidgetsGlobalTableActionsScopeHandler() {}
    WidgetsGlobalTableActionsScopeHandler(const FAction& action);

    WidgetsGlobalTableActionsScopeHandler& SetVisible(bool visible);

    LocalPropertyBool& Visibility() const;
    LocalPropertyBool& Enablity() const;
    TranslatedString& Text() const;
    WidgetsGlobalTableActionsScopeHandler& SetAction(const FAction& action);

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

    static WidgetsGlobalTableActionsScopeHandlersPtr EditHandlers(QTableView* table);
    static WidgetsGlobalTableActionsScopeHandlersPtr AddDefaultHandlers(QTableView* table);

private:
    friend class WidgetsGlobalTableActionId;
    QAction* registerAction(const Latin1Name& id, EnableIfMode mode = EIM_Default);
    virtual void CreateActions(){}

private:
    LocalPropertySharedPtr<WidgetsGlobalTableActionsScopeHandlers> m_currentHandlers;
    DispatcherConnectionsSafe m_currentHandlersConnections;
};

class WidgetsGlobalTableActionId : public Latin1Name
{
    using Super = Latin1Name;
public:
    WidgetsGlobalTableActionId(const char* id, WidgetsGlobalTableActionsScope::EnableIfMode mode);

private:
    friend class WidgetsGlobalTableActionsScope;
    static QVector<std::pair<Latin1Name, WidgetsGlobalTableActionsScope::EnableIfMode>> m_delayedRegistration;
};

#define DECLARE_GLOBAL_ACTION(ActionId) \
    const extern WidgetsGlobalTableActionId ActionId;

#define IMPLEMENT_GLOBAL_ACTION(ActionId, mode) \
    const WidgetsGlobalTableActionId ActionId(#ActionId, mode);

DECLARE_GLOBAL_ACTION(GlobalActionCopyId);
DECLARE_GLOBAL_ACTION(GlobalActionCopyWithHeadersId);
DECLARE_GLOBAL_ACTION(GlobalActionCutId);
DECLARE_GLOBAL_ACTION(GlobalActionPasteId);
DECLARE_GLOBAL_ACTION(GlobalActionDeleteId);
DECLARE_GLOBAL_ACTION(GlobalActionInsertId);

#endif // WIDGETSGLOBALTABLEACTIONSSCOPE_H
