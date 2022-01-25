#include "resource.h"

ResourceData::ResourceData(const FInitializer& initializer, const FReleaser& releaser)
    : m_data(nullptr)
    , m_initializer(initializer)
    , m_releaser(releaser)
    , m_useCount(0)
{}

ResourceData::~ResourceData()
{
    m_releaser(m_data);
}

void* ResourceData::use()
{
    lock();
    if(m_useCount++ == 0) {
        Q_ASSERT(m_data == nullptr);
        m_data = m_initializer();
    }
    unlock();
    return m_data;
}

void ResourceData::release()
{
    lock();
    if(--m_useCount == 0) {
        m_releaser(m_data);
        m_data = nullptr;
    }
    unlock();
}

void* ResourceData::get() const
{
    Q_ASSERT(m_data != nullptr);
    return m_data;
}

Resource::Resource(ResourceData* resourceData)
    : m_resourceData(resourceData)
{
    m_resourceData->use();
}

Resource::Resource(const Resource& resource)
    : m_resourceData(resource.m_resourceData)
{
    m_resourceData->use();
}

Resource::~Resource()
{
    m_resourceData->release();
}

