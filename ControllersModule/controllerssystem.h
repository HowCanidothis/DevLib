#ifndef CONTROLLERSSYSTEM_H
#define CONTROLLERSSYSTEM_H

#include <SharedModule/internal.hpp>

class ControllerBase;

class _Export ControllersSystem
{
    typedef std::function<void (ControllerBase*)> ControllerTask;
    ControllersSystem();
public:
    // Note: that's is delayed initialization. Use it for connections. It's never mind if controller is created or not.
    static void AddControllerTask(const Name& name, const ControllerTask& task);

    // Note: use this functions when you're sure that controller already exists
    static ControllerBase* GetController(const Name& name);
    template<class T>
    static T* GetController(const Name& name)
    {
        return  static_cast<T*>(GetController(name));
    }

private:
    friend class ControllerBase;

    static void registerController(const Name& name, ControllerBase* controller);
    static QHash<Name, QVector<ControllerTask>>& delayedTasks();
};

#endif // CONTROLLERSSYSTEM_H
