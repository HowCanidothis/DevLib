#ifndef ACTIONSSCOPEBASE_H
#define ACTIONSSCOPEBASE_H

#ifdef QT_GUI_LIB

#include <SharedModule/internal.hpp>

class Action;

class ActionsScopeBase : public QObject
{
public:
    ActionsScopeBase(const Latin1Name& scopeName);
    virtual ~ActionsScopeBase() {}

    virtual void CreateActions() = 0;

    const Latin1Name& GetName() const { return m_name; }
    QAction* FindAction(const Latin1Name& actionName) const;
    const Stack<QAction*>& GetActions() const { return m_actions; }
    QList<class QAction*> GetActionsQList() const;

    operator qint32() const { return m_name; }

protected:
    QAction* createAction(const Latin1Name& actionName, const FAction& action);

    Latin1Name GenerateFullActionName(const Latin1Name& actionName) const;

private:
    Latin1Name m_name;
    Stack<QAction*> m_actions;
    std::map<Latin1Name, QAction*> m_actionsMap;
};

class ActionsScope : public ActionsScopeBase
{
    using Super = ActionsScopeBase;
public:
    ActionsScope(const Latin1Name& scopeName)
        : Super(scopeName)
    {}

    void CreateActions() override {}

    QAction* CreateAction(const Latin1Name& actionName, const FAction& action) { return Super::createAction(actionName, action); }
};

#endif

#endif // IACTIONSSCOPE_H
