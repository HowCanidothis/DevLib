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
    Action* FindAction(const Latin1Name& actionName);
    const Stack<Action*>& GetActions() const { return m_actions; }
    QList<class QAction*> GetActionsQList() const;

    operator qint32() const { return m_name; }

protected:
    Action* createAction(const Latin1Name& actionName, const FAction& action);

    Latin1Name GenerateFullActionName(const Latin1Name& actionName) const;

private:
    Latin1Name m_name;
    Stack<Action*> m_actions;
    std::map<Latin1Name, Action*> m_actionsMap;
};

#endif

#endif // IACTIONSSCOPE_H
