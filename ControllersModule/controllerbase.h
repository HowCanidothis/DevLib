#ifndef CONTROLLERBASEV2_H
#define CONTROLLERBASEV2_H

#include <SharedModule/internal.hpp>

#include "controllerscontainer.h"

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
class DrawEngineBase;

class _Export ControllerBase : public QObject
{
    Q_OBJECT
protected:
    ControllersContainer* _container;
    ControllerBase* _parentController;
    Commands _commands;
    Name _name;
    QString _currentOperationName;

    StackPointers<ControllerBase> _childControllers;
public:
    ControllerBase(const Name& name, ControllersContainer* container, ControllerBase* parent=nullptr);
    virtual ~ControllerBase()
    {}

    void SetCurrent();
    void ResetCommandsChain(){ _commands.Clear(); }

    void Accept();
    void Cancel();
    template<class T> T* As() { return (T*)this; }
    template<class T> const T* As() const { return (const T*)this; }

    Commands* GetCommands() { return &_commands; }
    ControllerBase* GetParentController() const { return static_cast<ControllerBase*>(parent()); }

public Q_SLOTS:
    void Undo(){ _commands.Undo(); }
    void Redo(){ _commands.Redo(); }

protected:
    friend class ControllersContainer;

    void contextChanged();
    bool isCurrent() const;
    void setCurrent(const Name& controller);
    void setControllersContainer(ControllersContainer* container);
    template<class T> const T& context() const { return _container->GetContext<T>(); }
    template<class T> T& context() { return _container->GetContext<T>(); }

    virtual bool draw(DrawEngineBase*){ return false; }
    virtual bool mouseDoubleClickEvent(QMouseEvent* ){ return false; }
    virtual bool mouseMoveEvent(QMouseEvent* ){ return false; }
    virtual bool mousePressEvent(QMouseEvent* ){ return false; }
    virtual bool mouseReleaseEvent(QMouseEvent* ){ return false; }
    virtual bool wheelEvent(QWheelEvent* ){ return false; }
    virtual bool keyPressEvent(QKeyEvent* ){ return false; }
    virtual bool keyReleaseEvent(QKeyEvent* ){ return false; }
    virtual bool contextMenuEvent(QMenu* ){ return false; }
    virtual bool inputHandle(const QSet<qint32>*, qint32) { return false; }

    virtual void enterEvent() {}
    virtual void leaveEvent() {}

    virtual void pushCommandsToParentController(Commands* upLvlCommands);

    virtual void onContextChanged() {}
};

template<class T>
class ControllerContextedBase : public ControllerBase
{
    typedef ControllerBase Super;
public:
    using ControllerBase::ControllerBase;

protected:
    T& ctx() { return Super::context<T>(); }
    const T& ctx() const { return Super::context<T>(); }
};

#endif // CONTROLLERBASEV2_H
