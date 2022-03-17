#include "resourcessystem.h"
#include <QHash>

#include "resource.h"

ResourceData* ResourcesSystem::getResourceData(const Name& name) const
{
    const ResourceCache& cache = m_resources;
    auto find = cache.find(name);
    if(find == cache.end()) {
        return nullptr;
    }
    return find.value().get();
}

void ResourcesSystem::RegisterResource(const Name& name, const DescRegisterResourceParams& params)
{
    ResourceCache& cache = m_resources;
    auto find = cache.find(name);
    if(find == cache.end()) {
        SharedPointer<ResourceData> data;
        if(params.ThreadSafe) {
            data = ::make_shared<ResourceDataThreadSafe>(params.Initializer, params.Releaser);
        } else {
            data = ::make_shared<ResourceData>(params.Initializer, params.Releaser);
        }
        cache.insert(name, (data));
    }
    else {
        qCWarning(LC_SYSTEM) << QString("resource %1 already exists. Ignored").arg(name.AsString());
    }
}
