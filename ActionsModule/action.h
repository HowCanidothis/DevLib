#ifndef ACTION_H
#define ACTION_H

#include <QAction>

#include <SharedModule/internal.hpp>

class Action : public QAction
{
public:
    Action(const Name& actionName, const FAction& action);

    operator qint32() const { return m_name; }

private:
    Name m_name;
    FAction m_action;
};

#endif // ACTION_H
