#include "commands.h"
#include "memory.h"

void MacroCommand::Push(CommandBase* cmd)
{
    _commands.Append(cmd);
}

void MacroCommand::Redo()
{
    for(CommandBase* cmd : _commands)
        cmd->Redo();
}

void MacroCommand::Undo()
{
    for(CommandBase* command : adapters::reverse(_commands)) {
        command->Undo();
    }
}

CommandsContainer::CommandsContainer(capacity_type maxCommandsCount)
    : _commands(new CommandBase*[maxCommandsCount])
    , _count(0)
    , _undone(0)
    , _current(0)
    , _maxCommandsCount(maxCommandsCount)
    , _currentMacro(nullptr)
{
    Q_ASSERT((maxCommandsCount % 2) == 0);
}

CommandsContainer::~CommandsContainer()
{
    delete _commands;
}

void CommandsContainer::AddCommand(CommandBase* cmd)
{
    if(_currentMacro) {
        _currentMacro->Push(cmd);
    }
    else {
        addCommandPrivate(cmd);
    }
}

void CommandsContainer::AddCommandAndDo(CommandBase* cmd)
{
    AddCommand(cmd);
    cmd->Redo();
}

MacroCommand* CommandsContainer::ToMacro(const QString& name)
{
    MacroCommand* res = new MacroCommand(name);
    forEachBeforeCurrent([res](CommandBase* cmd){ res->Push(cmd); });
    forEachAfterCurrent([](CommandBase* cmd) { delete cmd; });
    reset();
    return res;
}

void CommandsContainer::Clear()
{
    forEach([](CommandBase* cmd){ delete cmd; });
    reset();
}

void CommandsContainer::BeginMacro(const QString& text)
{
    _currentMacro = new MacroCommand(text);
}

void CommandsContainer::EndMacro()
{
    CommandBase* tmp = _currentMacro;
    _currentMacro = nullptr;
    this->addCommandPrivate(tmp);
}

void CommandsContainer::Redo()
{
    if(CanRedo()) {
        redoPrivate();
    }
}

void CommandsContainer::Undo()
{
    if(CanUndo()){
        undoPrivate();
    }
}

void CommandsContainer::redoPrivate()
{
    _commands[_current]->Redo();
    increment(_current);
    _undone--;
}

void CommandsContainer::undoPrivate()
{
    decrement(_current);
    _commands[_current]->Undo();
    _undone++;
}

void CommandsContainer::addCommandPrivate(CommandBase* cmd)
{
    if(_undone != 0) {
        _count -= _undone;
        forEachAfterCurrent([](CommandBase* cmd){ delete cmd; });
        _undone = 0;
    }

    if(_count == _maxCommandsCount) {
        delete _commands[_current];
    } else {
        ++_count;
    }
    _commands[_current] = cmd;
    increment(_current);
}

capacity_type CommandsContainer::begin() const
{
    capacity_type res = _current - _count + _undone;
    res %= _maxCommandsCount;
    return res;
}

void CommandsContainer::reset()
{
    _count = 0;
    _undone = 0;
    _current = 0;
    _currentMacro = nullptr;
}

void CommandsContainer::forEachAfterCurrent(const std::function<void (CommandBase*)>& toDo)
{
    capacity_type current = _current;
    capacity_type undone = _undone;
    do {
        toDo(_commands[current]);
        increment(current);
    } while(--undone);
}

void CommandsContainer::forEachBeforeCurrent(const std::function<void (CommandBase*)>& toDo)
{
    capacity_type it = begin();
    while(it != _current) {
        toDo(_commands[it]);
        increment(it);
    }
}

void CommandsContainer::forEach(const std::function<void (CommandBase*)>& toDo)
{
    capacity_type it = begin();
    capacity_type count = GetCount();
    while(count--) {
        toDo(_commands[it]);
        increment(it);
    }
}

void CommandsContainer::increment(capacity_type& value) const
{
    value = (++value) % _maxCommandsCount;
}

void CommandsContainer::decrement(capacity_type& value) const
{
    value = (--value) % _maxCommandsCount;
}

capacity_type CommandsContainer::GetCount() const
{
    return _count;
}
