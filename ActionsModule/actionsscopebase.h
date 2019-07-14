#ifndef ACTIONSSCOPEBASE_H
#define ACTIONSSCOPEBASE_H

#include <SharedModule/internal.hpp>

class Action;

class ActionsScopeBase : public QObject
{
public:
    ActionsScopeBase(const Name& scopeName);

    virtual void CreateActions() = 0;

    const Stack<Action*>& GetActions() const { return m_actions; }

    operator qint32() const { return m_name; }

protected:
    Action* createAction(const QString& actionName, const FAction& action);

private:
    Name m_name;
    Stack<Action*> m_actions;
};

#endif // IACTIONSSCOPE_H
