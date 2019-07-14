#include "action.h"

Action::Action(const Name& actionName, const FAction& action)
    : m_name(actionName)
{
    connect(this, &QAction::triggered, [action]{
        action();
    });
}
