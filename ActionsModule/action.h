#ifndef ACTION_H
#define ACTION_H

#ifdef QT_GUI_LIB

#include <QAction>

#include <SharedModule/internal.hpp>

class Action : public QAction
{
public:
    explicit Action(const Latin1Name& actionName);
    Action(const Latin1Name& actionName, const FAction& action);

    void AddActionHandler(const FAction& action);

private:
    Latin1Name m_name;
    FAction m_action;
};

#endif

#endif // ACTION_H
