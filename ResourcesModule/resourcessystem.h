#ifndef RESOURCESSYSTEM_H
#define RESOURCESSYSTEM_H

#include <QString>
#include <functional>

#include "resource.h"
#include "multithreadresource.h"
#include <SharedModule/internal.hpp>

class ResourcesSystem
{
    ResourcesSystem() {}
    static ResourceBase* getResourceData(const Name& name);

public:
    static void RegisterResource(const Name& name, const std::function<void*()>& fOnCreate);

    template<class T>
    static Resource<T>* GetResource(const Name& name) {
        ResourceBase* data = getResourceData(name);
        if(data == nullptr) {
            qCWarning(LC_SYSTEM) << "trying to access undeclared resource" << name.AsString();
            return nullptr;
        }
        Resource<T>* result = new Resource<T>(data);
        return result;
    }

    template<class T>
    static MultiThreadResource<T>* GetMultiThreadResource(const Name& name) {
        ResourceBase* data = getResourceData(name);
        if(data == nullptr) {
            qCWarning(LC_SYSTEM) << "trying to access undeclared resource" << name.AsString();
            return nullptr;
        }
        MultiThreadResource<T>* result = new MultiThreadResource<T>(data);
        return result;
    }
};

#endif // RESOURCESSYSTEM_H
