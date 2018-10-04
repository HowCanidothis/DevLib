#ifndef RESOURCESSYSTEM_H
#define RESOURCESSYSTEM_H

#include <QString>
#include <functional>
#include <SharedModule/internal.hpp>

#include "resource.h"

class ResourcesSystem
{
    ResourcesSystem() {}
    static ResourceDataBase* getResourceData(const Name& name);

public:
    static void RegisterResource(const Name& name, const std::function<void*()>& fOnCreate, bool multiThread = false);

    template<class T>
    static Resource<T>* GetResource(const Name& name) {
        ResourceDataBase* data = getResourceData(name);
        if(data == nullptr) {
            qCWarning(LC_SYSTEM) << "trying to access undeclared resource" << name.AsString();
            return nullptr;
        }
        Resource<T>* result = new Resource<T>(data);
        return result;
    }
};

#endif // RESOURCESSYSTEM_H
