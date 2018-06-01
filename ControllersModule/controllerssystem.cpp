#include "controllerssystem.h"
#include "controllerscontainer.h"

#include <QHash>

static QHash<Name, ControllerBase*>& controllers()
{
    static QHash<Name, ControllerBase*> res;
    return res;
}

ControllersSystem::ControllersSystem()
{

}

ControllerBase* ControllersSystem::GetController(const Name& name)
{
    auto find = controllers().find(name);
    Q_ASSERT(find != controllers().end());
    return find.value();
}

void ControllersSystem::registerController(const Name& name, ControllerBase* controller)
{
    Q_ASSERT(!controllers().contains(name));
    controllers().insert(name, controller);
}
