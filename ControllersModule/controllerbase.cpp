#include "controllerbase.h"
#include "controllerssystem.h"

ControllerBase::ControllerBase(const Name& name, ControllersContainer* container, ControllerBase* parent)
    : QObject(parent)
    , _container(container)
    , _parentController(parent)
    , _name(name)
    , _currentOperationName("Undefined operation")
{
    Q_ASSERT(container);

    if(parent != nullptr) {
        parent->_childControllers.Append(this);
    } else {
        container->addMainController(this);
    }

    ControllersSystem::registerController(name, this);
}

void ControllerBase::SetCurrent()
{
    _container->SetCurrent(this);
}

void ControllerBase::Accept()
{
    if(auto parentController = GetParentController()) {
        leaveEvent();
        pushCommandsToParentController(parentController->GetCommands());
        parentController->enterEvent();
    }
}

void ControllerBase::Cancel()
{
    _commands.UndoAll();
    _commands.Clear();
    leaveEvent();
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

void ControllerBase::pushCommandsToParentController(Commands* upLvlCommands)
{
    if(!_commands.IsEmpty()) {
        upLvlCommands->AddCommand(_commands.ToMacro(_currentOperationName));
    }
}

void ControllerBase::contextChanged()
{
    for(ControllerBase* controller : _childControllers) {
        controller->contextChanged();
    }
    onContextChanged();
}

bool ControllerBase::isCurrent() const
{
    return _container->GetCurrent() == this;
}
