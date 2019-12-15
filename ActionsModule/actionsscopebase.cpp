#include "actionsscopebase.h"

#include "actionsmanager.h"

ActionsScopeBase::ActionsScopeBase(const Latin1Name& scopeName)
    : m_name(scopeName)
{
    ActionsManager::GetInstance().registerActionsScope(this);
}

Action* ActionsScopeBase::FindAction(const Latin1Name& actionName)
{
    auto found = m_actionsMap.find(actionName);
    if(found != m_actionsMap.end()) {
        return found->second;
    }
    return nullptr;
}

QList<QAction*> ActionsScopeBase::GetActionsQList() const
{
    QList<QAction*> result;
    for(auto* action : m_actions) {
        result.append(action);
    }
    return result;
}

Action* ActionsScopeBase::createAction(const Latin1Name& actionName, const FAction& action)
{
    auto* result = ActionsManager::GetInstance().createAction(Latin1Name(GenerateFullActionName(actionName)), action);
    m_actions.Append(result);
    m_actionsMap.insert(std::make_pair(actionName, result));
    return result;
}

Latin1Name ActionsScopeBase::GenerateFullActionName(const Latin1Name& actionName) const
{
    return m_name.AsLatin1String() + "." + actionName.AsLatin1String();
}
