#ifndef RESOURCESSYSTEM_H
#define RESOURCESSYSTEM_H

#include <QString>
#include <functional>
#include <SharedModule/internal.hpp>

#include "resource.h"

class ResourcesSystem
{
    typedef QHash<Name, ResourceDataBase*> ResourceCache;
    ResourceDataBase* getResourceData(const Name& name);

public:
    ResourcesSystem() {}
    void RegisterResource(const Name& name, const std::function<void*()>& fOnCreate, bool multiThread = false);

    template<class T>
    SharedPointer<Resource<T>> GetResource(const Name& name, bool silent = false) {
        ResourceDataBase* data = getResourceData(name);
        if(data == nullptr) {
            if(!silent) {
                qCWarning(LC_SYSTEM) << "trying to access undeclared resource" << name;
            }
            return nullptr;
        }
        return ::make_shared<Resource<T>>(data);
    }

private:
    ResourceCache m_resources;
};

class ResourcesSystemCurrentGuard
{
public:
    ResourcesSystemCurrentGuard(ResourcesSystem* storage);
    ~ResourcesSystemCurrentGuard();

    static ResourcesSystem* GetCurrent() { return current(); }

private:
    static ResourcesSystem*& current();
    ResourcesSystem* m_previous;
};

#endif // RESOURCESSYSTEM_H
