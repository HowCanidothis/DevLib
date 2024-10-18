#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#include <SharedModule/internal.hpp>

class ActionsScopeBase;

class ActionsManager : public SingletoneGlobal<ActionsManager>
{
    template<class T> friend class SingletoneGlobal;
    ActionsManager();
public:

    void Initialize();

    void CreateActionsFromRegisteredScopes();
    ActionsScopeBase* FindScope(const Latin1Name& scopeName) const;
    template<class T>
    T* FindScope(const Latin1Name& scopeName) const
    {
        return reinterpret_cast<T*>(FindScope(scopeName));
    }
    bool AddTo(const Latin1Name& scopeName, class QMenu* menu);
    bool AddTo(const Latin1Name& scopeName, QWidget* widget);
    class QAction* FindAction(const QString& path) const;

private:
    friend class ActionsScopeBase;
    void registerActionsScope(ActionsScopeBase* actionsScope);
    QAction* createAction(const Latin1Name& actionName, const FAction& action);

private:
    std::map<Latin1Name, QAction*> m_actions;
    std::map<Latin1Name, ActionsScopeBase*> m_actionsScopes;
};

#endif // ACTIONSMANAGER_H
