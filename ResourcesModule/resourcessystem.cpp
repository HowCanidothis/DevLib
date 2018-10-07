#include "resourcessystem.h"
#include <QHash>

#include "resource.h"

typedef QHash<Name, ResourceDataBase*> ResourceCache;

static ResourceCache& getResourcesCache() {
    static ResourceCache result;
    return result;
}

ResourceDataBase* ResourcesSystem::getResourceData(const Name& name)
{
    ResourceCache& cache = getResourcesCache();
    auto find = cache.find(name);
    if(find == cache.end()) {
        return nullptr;
    }
    return find.value();
}

void ResourcesSystem::RegisterResource(const Name& name, const std::function<void*()>& fOnCreate, bool multiThread)
{
    ResourceCache& cache = getResourcesCache();
    auto find = cache.find(name);
    if(find == cache.end()) {
        ResourceDataBase* data;
        if(multiThread) {
            data = new ResourceDataMultiThread(fOnCreate);
        } else {
            data = new ResourceDataBase(fOnCreate);
        }
        cache.insert(name,data);
    }
    else {
        qCWarning(LC_SYSTEM) << QString("resource %1 already exists. Ignored").arg(name.AsString());
    }
}
