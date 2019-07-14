#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#include <SharedModule/internal.hpp>

#include "action.h"

class ActionsScopeBase;

class ActionsManager
{
    ActionsManager();
public:
    static ActionsManager& GetInstance();

private:
    friend class ActionsScopeBase;
    void registerActionsScope(ActionsScopeBase& actionsScope);
    Action* createAction(const Name& actionName, const FAction& action);

private:
    std::set<Action> m_actions;
    std::set<NamedClassReferenceWrapper<ActionsScopeBase>> m_actionsScopes;
};

#endif // ACTIONSMANAGER_H
