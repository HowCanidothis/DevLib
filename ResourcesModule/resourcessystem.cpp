#include "resourcessystem.h"
#include <QHash>

#include "resource.h"

ResourceDataBase* ResourcesSystem::getResourceData(const Name& name)
{
    ResourceCache& cache = m_resources;
    auto find = cache.find(name);
    if(find == cache.end()) {
        return nullptr;
    }
    return find.value();
}

void ResourcesSystem::RegisterResource(const Name& name, const std::function<void*()>& fOnCreate, bool multiThread)
{
    ResourceCache& cache = m_resources;
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

ResourcesSystemCurrentGuard::ResourcesSystemCurrentGuard(ResourcesSystem* storage)
{
    THREAD_ASSERT_IS_MAIN();
    m_previous = current();
    current() = storage;
}

ResourcesSystemCurrentGuard::~ResourcesSystemCurrentGuard()
{
    current() = m_previous;
}

ResourcesSystem*& ResourcesSystemCurrentGuard::current()
{
    static ResourcesSystem* current = nullptr;
    return current;
}
