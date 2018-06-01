#include "controllerbase.h"
#include "controllerssystem.h"

ControllerBase::ControllerBase(const Name& name, ControllersContainer* container, ControllerBase* parent)
    : QObject(parent)
    , _container(container)
    , _parentController(parent)
    , _name(name)
{
    Q_ASSERT(container);

    if(parent != nullptr) {
        parent->_childControllers.Append(this);
    } else {
        container->addMainController(this);
    }

    ControllersSystem::registerController(name, this);
}

void ControllerBase:: SetCurrent()
{
    _container->SetCurrent(this);
}

void ControllerBase::setCurrent(const Name& controller)
{
    Q_ASSERT(_container != nullptr);
    _container->SetCurrent(controller);
}

void ControllerBase::setControllersContainer(ControllersContainer* container)
{
    Q_ASSERT(_container == nullptr);
    _container = container;
}

void ControllerBase::contextChanged()
{
    for(ControllerBase* controller : _childControllers) {
        controller->contextChanged();
    }
    onContextChanged();
}
