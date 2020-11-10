#include "controllerbase.h"

ControllerBase::ControllerBase(const Name& name, ControllersContainer* container, ControllerBase* parent)
    : QObject(parent)
    , m_container(container)
    , m_parentController(parent)
    , m_name(name)
    , m_currentOperationName("Undefined operation")
{
    Q_ASSERT(container);

    if(parent != nullptr) {
        parent->m_childControllers.Append(this);
    } else {
        container->addMainController(this);
    }

    container->registerController(name, this);
}

void ControllerBase::SetCurrent()
{
    m_container->SetCurrent(this);
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
    Q_ASSERT(m_container != nullptr);
    m_container->SetCurrent(controller);
}

void ControllerBase::setControllersContainer(ControllersContainer* container)
{
    Q_ASSERT(m_container == nullptr);
    m_container = container;
}

void ControllerBase::pushCommandsToParentController(Commands* upLvlCommands)
{
    if(!_commands.IsEmpty()) {
        upLvlCommands->AddCommand(_commands.ToMacro(m_currentOperationName));
    }
}

void ControllerBase::contextChanged()
{
    for(ControllerBase* controller : m_childControllers) {
        controller->contextChanged();
    }
    onContextChanged();
}

bool ControllerBase::isCurrent() const
{
    return m_container->GetCurrent() == this;
}
