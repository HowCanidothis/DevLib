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

void ActionsManager::CreateActionsFromRegisteredScopes()
{
    for(const auto& it : m_actionsScopes) {
        it.second->CreateActions();
    }
}

ActionsScopeBase* ActionsManager::FindScope(const Latin1Name& scopeName)
{
    auto found = m_actionsScopes.find(scopeName);
    if(found != m_actionsScopes.end()) {
        return found->second;
    }
    return nullptr;
}

void ActionsManager::registerActionsScope(ActionsScopeBase* actionsScope)
{
    Q_ASSERT(m_actionsScopes.find(actionsScope->GetName()) == m_actionsScopes.end());

    m_actionsScopes.insert(std::make_pair(actionsScope->GetName(), actionsScope));
}

Action* ActionsManager::createAction(const Latin1Name& actionName, const FAction& action)
{
    const auto& it = m_actions.emplace(actionName, new Action(actionName, action)).first;
    return it->second;
}
