#ifndef CONTROLLERSCONTAINER_H
#define CONTROLLERSCONTAINER_H

#include <SharedModule/internal.hpp>

class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QMenu;
class DrawEngineBase;
class ControllerBase;

class ControllersContainer: public QObject
{
    typedef Array<ControllerBase*> Controllers;
public:
    ControllersContainer(QObject* parent=0);
    ~ControllersContainer();

    template<class T>
    void SetContext(T* context)
    {
        _context = new ControllersContext<T>();
        _context->Data = context;
        for(ControllerBase* controller : _controllers) {
            controller->contextChanged();
        }
    }

    void SetCurrent(ControllerBase* controller);
    void SetCurrent(const Name& name);
    ControllerBase* GetCurrent() const { return _currentController; }
    template<class T> T& GetContext() { return _context->As<T>(); }
    template<class T> const T& GetContext() const { return _context->As<T>(); }

    void Accept();
    void Abort();
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
    qint32& getInputKeysModifiers() { return _context->InputKeysModifiers; }
    QSet<qint32>& getInputKeys() { return _context->InputKeys; }

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

    struct ControllersContextBase
    {
        void* Data;

        qint32 InputKeysModifiers;
        QSet<qint32> InputKeys;

        template<class T> T& As() { return *(T*)Data; }
        template<class T> const T& As() const { return *(T*)Data; }

        ControllersContextBase(){}
        virtual ~ControllersContextBase(){}
    };

    template<class T>
    struct ControllersContext : ControllersContextBase
    {
        virtual ~ControllersContext() {
            delete (T*)Data;
        }
    };

private:
    StackPointers<ControllerBase> _controllers;
    ControllerBase* _currentController;
    ScopedPointer<ControllersContextBase> _context;
};

#endif // CONTROLLERSCONTAINER_H
