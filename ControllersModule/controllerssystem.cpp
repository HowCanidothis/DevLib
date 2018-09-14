#include "controllerssystem.h"
#include "controllerscontainer.h"

#include <QHash>

static QHash<Name, ControllerBase*>& controllers()
{
    static QHash<Name, ControllerBase*> res;
    return res;
}

ControllersSystem::ControllersSystem()
{

}

void ControllersSystem::AddControllerTask(const Name& name, const ControllersSystem::ControllerTask& task)
{
    auto find = controllers().find(name);
    if(find == controllers().end()) {
        delayedTasks()[name].append(task);
    } else {
        task(find.value());
    }
}

ControllerBase* ControllersSystem::GetController(const Name& name)
{
    auto find = controllers().find(name);
    Q_ASSERT(find != controllers().end());
    return find.value();
}

void ControllersSystem::registerController(const Name& name, ControllerBase* controller)
{
    Q_ASSERT(!controllers().contains(name));
    controllers().insert(name, controller);
    auto hasTasks = delayedTasks().find(name);
    if(hasTasks != delayedTasks().end()) {
        for(const auto& task : hasTasks.value()) {
            task(controller);
        }
    }
}

QHash<Name, QVector<ControllersSystem::ControllerTask>>& ControllersSystem::delayedTasks()
{
    static QHash<Name, QVector<ControllersSystem::ControllerTask>> ret;
    return ret;
}
