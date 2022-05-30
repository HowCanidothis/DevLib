#include "actionsmanager.h"

#ifdef QT_GUI_LIB

#include <QMenu>

#include "actionsscopebase.h"

ActionsManager::ActionsManager()
{

}

void ActionsManager::CreateActionsFromRegisteredScopes()
{
    for(const auto& it : m_actionsScopes) {
        it.second->CreateActions();
    }
}

ActionsScopeBase* ActionsManager::FindScope(const Latin1Name& scopeName) const
{
    auto found = m_actionsScopes.find(scopeName);
    if(found != m_actionsScopes.end()) {
        return found->second;
    }
    return nullptr;
}

bool ActionsManager::AddTo(const Latin1Name& scopeName, QMenu* menu)
{
    auto* scope = FindScope(scopeName);
    if(scope != nullptr) {
        for(auto* action : scope->GetActions()) {
            menu->addAction(action);
        }

        return true;
    }

    return false;
}

bool ActionsManager::AddTo(const Latin1Name& scopeName, QWidget* widget)
{
    auto* scope = FindScope(scopeName);
    if(scope != nullptr) {
        widget->addActions(scope->GetActionsQList());

        return true;
    }

    return false;
}

QAction* ActionsManager::FindAction(const QString& path) const
{
    auto splittedPath = path.split(".");
    Q_ASSERT(splittedPath.size() == 2);
    auto* scope = FindScope(Latin1Name(splittedPath.first().toLatin1()));
    if(scope == nullptr) {
        return nullptr;
    }
    return scope->FindAction(Latin1Name(splittedPath.last().toLatin1()));
}

void ActionsManager::registerActionsScope(ActionsScopeBase* actionsScope)
{
    Q_ASSERT(m_actionsScopes.find(actionsScope->GetName()) == m_actionsScopes.end());

    m_actionsScopes.insert(std::make_pair(actionsScope->GetName(), actionsScope));
}

QAction* ActionsManager::createAction(const Latin1Name& actionName, const FAction& action)
{
    auto* newAction = new QAction();
    newAction->connect(newAction, &QAction::triggered, action);
    const auto& it = m_actions.emplace(actionName, newAction).first;
    return it->second;
}

#endif
