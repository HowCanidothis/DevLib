#ifndef COMMANDS_H
#define COMMANDS_H

#include <functional>
#include <QObject>
#include <SharedModule/stack.h>

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
    CommandsContainer(capacity_type maxCommandsCount = 0xfffe);
    ~CommandsContainer();
    void AddCommand(CommandBase* cmd);
    void AddCommandAndDo(CommandBase* cmd);

    MacroCommand* ToMacro(const QString& name);

    void Clear();

    void BeginMacro(const QString &text);
    void EndMacro();

    bool CanRedo() const { return _undone != 0; }
    bool CanUndo() const { return _undone != _count; }

    qint32 GetCurrentIndex() const { return _count - _undone - 1; }
    const CommandBase* GetCommand(capacity_type i) { return _commands[(begin() + i) % _maxCommandsCount]; }
    capacity_type GetCount() const;

public Q_SLOTS:
    void Redo();
    void Undo();

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
