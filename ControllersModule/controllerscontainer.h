#ifndef CONTROLLERSCONTAINER_H
#define CONTROLLERSCONTAINER_H

#include <Shared/internal.hpp>

class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QMenu;
class DrawEngineBase;
class ControllerBase;

class ControllersContainer: public QObject
{
    typedef Array<ControllerBase*> Controllers;
    StackPointers<ControllerBase> _controllers;
    ControllerBase* _currentController;
public:
    ControllersContainer(QObject* parent=0);
    ~ControllersContainer();

    template<class T>
    T* AddMainController(T* controller)
    {
        Q_ASSERT(controller->GetParentController() == nullptr);
        _controllers.Append(controller);
        _currentController = (_currentController == nullptr) ? controller : _currentController;
        return controller;
    }

    void SetCurrent(ControllerBase* controller);
    void SetCurrent(const Name& name);
    ControllerBase* GetCurrent() const { return _currentController; }

    void Accept();
    void Abort();
    void Undo();
    void Redo();

    void Draw(DrawEngineBase* );
    void MouseMoveEvent(QMouseEvent* );
    void MousePressEvent(QMouseEvent* );
    void MouseReleaseEvent(QMouseEvent* );
    void MouseDoubleClickEvent(QMouseEvent* );
    void WheelEvent(QWheelEvent* );
    void KeyPressEvent(QKeyEvent* );
    void KeyReleaseEvent(QKeyEvent* );
    void ContextMenuEvent(QMenu* );

private:
    ControllerBase* findCommonParent(ControllerBase* c1, ControllerBase* c2) const;
    Controllers findAllParents(ControllerBase* c) const;

    // CurrentController call function, if function return false call parentController(if has) function and so on
    template<typename ... Args>
    void callFunctionRecursivly(bool (ControllerBase::*function)(Args...), Args ... args)
    {
        Q_ASSERT(_currentController);
        if(!(_currentController->*function)(args...)) {
            ControllerBase* parent;
            ControllerBase* current = _currentController;
            while((parent = current->GetParentController()) &&
                  (parent->*function)(args...) == false) {
                current = parent;
            }
        }
    }
};

#endif // CONTROLLERSCONTAINER_H
