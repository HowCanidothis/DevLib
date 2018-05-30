#ifndef CONTROLLERBASEV2_H
#define CONTROLLERBASEV2_H

#include <Shared/stack.h>

#ifdef VISUAL_COMMANDS
#include "CommandsModule/commandsvisualcomponents.h"
typedef CommandsContainerVisual Commands;
#else
#include "CommandsModule/commands.h"
typedef CommandsContainer Commands;
#endif
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QMenu;
class ControllersContainer;
class DrawEngineBase;

class ControllerBase : public QObject
{
protected:
    ControllerBase* _parentController;
    Commands _commands;
    QString _name;

    StackPointers<ControllerBase> _childControllers;
public:
    ControllerBase(const QString& name, ControllerBase* parent=0)
        : QObject(parent)
        , _parentController(nullptr)
        , _name(name)
    {
        if(parent != nullptr) {
            parent->_childControllers.Append(this);
        }
    }
    virtual ~ControllerBase()
    {}

    void ResetCommandsChain(){ _commands.Clear(); }

    virtual void Accept(Commands* upLvlCommands) { Q_UNUSED(upLvlCommands) }
    virtual void Abort(){ }


    Commands* GetCommands() { return &_commands; }
    ControllerBase* GetParentController() const { return static_cast<ControllerBase*>(parent()); }

public Q_SLOTS:
    void Undo(){ _commands.Undo(); }
    void Redo(){ _commands.Redo(); }

protected:
    friend class ControllersContainer;

    virtual bool Draw(DrawEngineBase*){ return false; }
    virtual bool mouseDoubleClickEvent(QMouseEvent *){ return false; }
    virtual bool mouseMoveEvent(QMouseEvent *){ return false; }
    virtual bool mousePressEvent(QMouseEvent *){ return false; }
    virtual bool mouseReleaseEvent(QMouseEvent *){ return false; }
    virtual bool wheelEvent(QWheelEvent *){ return false; }
    virtual bool keyPressEvent(QKeyEvent *){ return false; }
    virtual bool keyReleaseEvent(QKeyEvent *){ return false; }
    virtual bool contextMenuEvent(QMenu *){ return false; }
};

#endif // CONTROLLERBASEV2_H
