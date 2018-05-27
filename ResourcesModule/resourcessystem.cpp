#include "resourcessystem.h"
#include <QHash>

#include "resource.h"

typedef QHash<Name, ResourceBase*> ResourceCache;

static ResourceCache& getResourcesCache() {
    static ResourceCache result;
    return result;
}

ResourceBase* ResourcesSystem::getResourceData(const Name& name)
{
    ResourceCache& cache = getResourcesCache();
    auto find = cache.find(name);
    if(find == cache.end()) {
        return nullptr;
    }
    return find.value();
}

void ResourcesSystem::RegisterResource(const Name& name, const std::function<void*()>& fOnCreate)
{
    LOGOUT;
    ResourceCache& cache = getResourcesCache();
    auto find = cache.find(name);
    if(find == cache.end()) {
        ResourceBase* resource = new ResourceBase(fOnCreate);
        cache.insert(name,resource);
    }
    else {
        log.Warning() << "resource" << name.AsString() << "already exists. Ignored";
    }
}
