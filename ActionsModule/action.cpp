#include "action.h"

#ifdef QT_GUI_LIB

Action::Action(const Latin1Name& actionName)
    : m_name(actionName)
{

}

Action::Action(const Latin1Name& actionName, const FAction& action)
    : m_name(actionName)
{
    connect(this, &QAction::triggered, [action]{
        action();
    });
}

void Action::AddActionHandler(const FAction& action)
{
    connect(this, &QAction::triggered, [action]{
        action();
    });
}

#endif
