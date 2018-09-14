#ifndef CONTROLLERSCONTAINER_H
#define CONTROLLERSCONTAINER_H

#include <SharedModule/internal.hpp>

class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QMenu;
class DrawEngineBase;
class ControllerBase;

class _Export ControllersContainer: public QObject
{
    typedef Array<ControllerBase*> Controllers;
public:
    ControllersContainer(QObject* parent=0);
    ~ControllersContainer();

    template<class T>
    void SetContext(T* context)
    {
        _context = context;
        for(ControllerBase* controller : _controllers) {
            controller->contextChanged();
        }
    }

    void SetCurrent(ControllerBase* controller);
    void SetCurrent(const Name& name);
    ControllerBase* GetCurrent() const { return _currentController; }
    bool HasContext() const { return _context != nullptr; }
    template<class T> T& GetContext() { Q_ASSERT(_context != nullptr); return *static_cast<T*>(_context); }
    template<class T> const T& GetContext() const { Q_ASSERT(_context != nullptr); return *static_cast<T*>(_context); }

    void Accept();
    void Cancel();
    void Undo();
    void Redo();

    void Input();
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
    friend class ControllerBase;
    qint32& getInputKeysModifiers() { return _inputKeysModifiers; }
    QSet<qint32>& getInputKeys() { return _inputKeys; }

    ControllerBase* findCommonParent(ControllerBase* c1, ControllerBase* c2) const;
    Controllers findAllParents(ControllerBase* c) const;

    void addMainController(ControllerBase* controller);

    // Calls currentController's function, if the function has returned false then calls parentController's(if has) function and so on
    template<typename ... Args>
    void callFunctionRecursively(bool (ControllerBase::*function)(Args...), Args ... args)
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

private:
    StackPointers<ControllerBase> _controllers;
    ControllerBase* _currentController;
    void* _context;
    qint32 _inputKeysModifiers;
    QSet<qint32> _inputKeys;
};

#endif // CONTROLLERSCONTAINER_H
