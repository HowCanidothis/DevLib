#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#ifdef QT_GUI_LIB

#include <SharedModule/internal.hpp>

#include "action.h"

class ActionsScopeBase;

class ActionsManager
{
    ActionsManager();
public:
    static ActionsManager& GetInstance();

    void CreateActionsFromRegisteredScopes();
    ActionsScopeBase* FindScope(const Latin1Name& scopeName) const;
    bool AddTo(const Latin1Name& scopeName, QMenu* menu);
    bool AddTo(const Latin1Name& scopeName, QWidget* widget);
    Action* FindAction(const QString& path) const;

private:
    friend class ActionsScopeBase;
    void registerActionsScope(ActionsScopeBase* actionsScope);
    Action* createAction(const Latin1Name& actionName, const FAction& action);

private:
    std::map<Latin1Name, Action*> m_actions;
    std::map<Latin1Name, ActionsScopeBase*> m_actionsScopes;
};

#endif

#endif // ACTIONSMANAGER_H
