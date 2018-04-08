#ifndef RESOURCESSYSTEM_H
#define RESOURCESSYSTEM_H

#include <QString>
#include <functional>

#include "resource.h"
#include "Shared/shared_decl.h"

class ResourcesSystem
{
    ResourcesSystem() {}


    static ResourceBase* getResourceData(const QString& name);
public:
    static void registerResource(const QString& name, const std::function<void*()>& on_create);

    template<class T> static Resource<T>* getResource(const QString& name) {
        ResourceBase* data = getResourceData(name);
        if(data == nullptr) {
            ASSURE(false, "trying to access undeclared resource", name.toLatin1().data())
            return nullptr;
        }
        Resource<T>* result = new Resource<T>(data);
        return result;
    }
};

#endif // RESOURCESSYSTEM_H
