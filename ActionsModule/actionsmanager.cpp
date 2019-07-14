#include "actionsmanager.h"

#include "actionsscopebase.h"

ActionsManager::ActionsManager()
{

}

ActionsManager& ActionsManager::GetInstance()
{
    static ActionsManager result;
    return result;
}

void ActionsManager::registerActionsScope(ActionsScopeBase& actionsScope)
{
    Q_ASSERT(m_actionsScopes.find(actionsScope) == m_actionsScopes.end());

    m_actionsScopes.insert(actionsScope);
}

Action* ActionsManager::createAction(const Name& actionName, const FAction& action)
{
    auto& it = m_actions.emplace(actionName, action).first;
    return const_cast<Action*>(&*it);
}
