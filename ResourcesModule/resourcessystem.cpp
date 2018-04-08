#include "resourcessystem.h"
#include <QHash>

#include "resource.h"

typedef QHash<QString, ResourceBase*> ResourceCache;

static ResourceCache& getResourcesCache() {
    static ResourceCache result;
    return result;
}

ResourceBase* ResourcesSystem::getResourceData(const QString& name)
{
    ResourceCache& cache = getResourcesCache();
    auto find = cache.find(name);
    if(find == cache.end()) {
        return nullptr;
    }
    return find.value();
}

void ResourcesSystem::registerResource(const QString& name, const std::function<void*()>& on_create)
{
    ResourceCache& cache = getResourcesCache();
    auto find = cache.find(name);
    if(find == cache.end()) {
        ResourceBase* resource = new ResourceBase(on_create);
        cache.insert(name,resource);
    }
    else {
        find.value()->setLoader(on_create);
    }
}
