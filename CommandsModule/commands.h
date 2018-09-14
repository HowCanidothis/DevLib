#ifndef COMMANDS_H
#define COMMANDS_H

#include <functional>
#include <QObject>
#include <SharedModule/internal.hpp>

typedef quint16 capacity_type;

class CommandBase
{
protected:
    QString _text;
public:
    CommandBase(const QString& t="") : _text(t) {}
    virtual ~CommandBase(){}
    virtual void Redo()=0;
    virtual void Undo()=0;

    const QString& GetText() const { return _text; }
};

class MacroCommand : public CommandBase
{
    StackPointers<CommandBase> _commands;
public:
    MacroCommand(const QString& t)
        : CommandBase(t)
    {}
    ~MacroCommand()
    {}
    void Push(CommandBase* cmd);

    // ICommand interface
public:
    virtual void Redo() Q_DECL_OVERRIDE;
    virtual void Undo() Q_DECL_OVERRIDE;
};

class CommandsContainer : public QObject
{
    Q_OBJECT
protected:
    CommandBase** _commands;
    capacity_type _current;
    capacity_type _count;
    capacity_type _undone;
    capacity_type _maxCommandsCount;
    MacroCommand* _currentMacro;
public:
    _Export CommandsContainer(capacity_type maxCommandsCount = 0xfffe);
    _Export ~CommandsContainer();
    _Export void AddCommand(CommandBase* cmd);
    _Export void AddCommandAndDo(CommandBase* cmd);

    _Export MacroCommand* ToMacro(const QString& name);

    _Export void Clear();
    bool IsEmpty() const { return _count == 0; }

    _Export void BeginMacro(const QString &text);
    _Export void EndMacro();

    bool CanRedo() const { return _undone != 0; }
    bool CanUndo() const { return _undone != _count; }

    qint32 GetCurrentIndex() const { return _count - _undone - 1; }
    const CommandBase* GetCommand(capacity_type i) { return _commands[(begin() + i) % _maxCommandsCount]; }
    _Export capacity_type GetCount() const;

public Q_SLOTS:
    _Export void Redo();
    _Export void Undo();
    _Export void UndoAll();

protected: //common element for visualable commands container
    void redoPrivate();
    void undoPrivate();
    void addCommandPrivate(CommandBase* c);
    capacity_type begin() const;
    void reset();
    void forEachAfterCurrent(const std::function<void (CommandBase*)>& toDo);
    void forEachBeforeCurrent(const std::function<void (CommandBase*)>& toDo);
    void forEach(const std::function<void (CommandBase* command)>& toDo);
    void increment(capacity_type& value) const;
    void decrement(capacity_type& value) const;
};

#endif // FIXEDCONTAINER_H
