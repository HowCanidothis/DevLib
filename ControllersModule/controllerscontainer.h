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
        m_context = context;
        for(ControllerBase* controller : m_controllers) {
            controller->contextChanged();
        }
    }

    void SetVisibilityMask(qint32 vm);
    void SetCurrent(ControllerBase* controller);
    void SetCurrent(const Name& name);
    ControllerBase* GetCurrent() const { return m_currentController; }
    bool HasContext() const { return m_context != nullptr; }
    template<class T> T& GetContext() { Q_ASSERT(m_context != nullptr); return *static_cast<T*>(m_context); }
    template<class T> const T& GetContext() const { Q_ASSERT(m_context != nullptr); return *static_cast<T*>(m_context); }
    qint32 GetVisibilityMask() const { return m_visibilityMask; }

    void Accept();
    void Cancel();

    void Input();
    void Draw(DrawEngineBase* );
    void MouseMoveEvent(QMouseEvent* );
    void MousePressEvent(QMouseEvent* );
    void MouseReleaseEvent(QMouseEvent* );
    void MouseDoubleClickEvent(QMouseEvent* );
    void OnClicked(QMouseEvent*);
    void WheelEvent(QWheelEvent* );
    void KeyPressEvent(QKeyEvent* );
    void KeyReleaseEvent(QKeyEvent* );
    void ContextMenuEvent(QMenu* );
    void LeaveEvent();

private:
    friend class ControllerBase;
    qint32& getInputKeysModifiers() { return m_inputKeysModifiers; }
    QSet<qint32>& getInputKeys() { return m_inputKeys; }

    ControllerBase* findCommonParent(ControllerBase* c1, ControllerBase* c2) const;
    Controllers findAllParents(ControllerBase* c) const;

    void addMainController(ControllerBase* controller);
    void registerController(const Name& name, ControllerBase* controller);

    // Calls currentController's function, if the function has returned false then calls parentController's(if has) function and so on
    template<typename ... Args>
    void callFunctionRecursively(bool (ControllerBase::*function)(Args...), Args ... args)
    {
        Q_ASSERT(m_currentController);
        if(!(m_currentController->*function)(args...)) {
            ControllerBase* parent;
            ControllerBase* current = m_currentController;
            while((parent = current->GetParentController()) &&
                  (parent->*function)(args...) == false) {
                current = parent;
            }
        }
    }

private:
    QHash<Name, ControllerBase*> m_controllersMap;
    StackPointers<ControllerBase> m_controllers;
    ControllerBase* m_currentController;
    void* m_context;
    qint32 m_inputKeysModifiers;
    QSet<qint32> m_inputKeys;
    qint32 m_visibilityMask;
};

#endif // CONTROLLERSCONTAINER_H
