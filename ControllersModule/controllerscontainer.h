#ifndef CONTROLLERSCONTAINER_H
#define CONTROLLERSCONTAINER_H

#include <Shared/stack.h>
#include <Shared/array.h>

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
    ControllerBase *_currentParent;
    ControllerBase *_currentController;
public:

    template<class T>
    void AddMainController(T* controller)
    {
        Q_ASSERT(controller->GetParentController() == nullptr);
        _controllers.Append(controller);
    }

    void SetCurrent(ControllerBase* controller);
    ControllerBase* getCurrent() const { return _currentController; }

    void Accept();
    void Abort();
    void undo();
    void redo();

    void draw(DrawEngineBase* );
    void mouseMoveEvent(QMouseEvent* );
    void mousePressEvent(QMouseEvent* );
    void mouseReleaseEvent(QMouseEvent* );
    void mouseDoubleClickEvent(QMouseEvent* );
    void wheelEvent(QWheelEvent* );
    void keyPressEvent(QKeyEvent* );
    void keyReleaseEvent(QKeyEvent* );
    void contextMenuEvent(QMenu* );

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
