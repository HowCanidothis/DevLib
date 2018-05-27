#ifndef RESOURCESSYSTEM_H
#define RESOURCESSYSTEM_H

#include <QString>
#include <functional>

#include "resource.h"
#include "multithreadresource.h"
#include "Shared/shared_decl.h"

class ResourcesSystem
{
    ResourcesSystem() {}
    static ResourceBase* getResourceData(const Name& name);

public:
    static void registerResource(const Name& name, const std::function<void*()>& on_create);

    template<class T> static Resource<T>* getResource(const Name& name) {
        ResourceBase* data = getResourceData(name);
        if(data == nullptr) {
            ASSURE(false, "trying to access undeclared resource", name.AsString().toLatin1().data())
            return nullptr;
        }
        Resource<T>* result = new Resource<T>(data);
        return result;
    }

    template<class T> static MultiThreadResource<T>* getMultiThreadResource(const Name& name) {
        ResourceBase* data = getResourceData(name);
        if(data == nullptr) {
            ASSURE(false, "trying to access undeclared resource", name.AsString().toLatin1().data())
            return nullptr;
        }
        MultiThreadResource<T>* result = new MultiThreadResource<T>(data);
        return result;
    }
};

#endif // RESOURCESSYSTEM_H
