#include "controllerbase.h"
#include "controllerssystem.h"

ControllerBase::ControllerBase(const Name& name, ControllerBase* parent)
    : QObject(parent)
    , _parentController(parent)
    , _name(name)
{
    if(parent != nullptr) {
        parent->_childControllers.Append(this);
    }

    ControllersSystem::registerController(name, this);
}
