#include "actionsscopebase.h"

#include "actionsmanager.h"

ActionsScopeBase::ActionsScopeBase(const Name& scopeName)
    : m_name(scopeName)
{
    ActionsManager::GetInstance().registerActionsScope(*this);
}

Action* ActionsScopeBase::createAction(const QString& actionName, const FAction& action)
{
    auto resultName = QString("%1.%2").arg(m_name.AsString(), actionName);
    auto* result = ActionsManager::GetInstance().createAction(Name(resultName), action);
    m_actions.Append(result);
    return result;
}
