#include "resource.h"

Resource::Resource(const ResourceDataCapture& resourceData)
    : m_resourceData(resourceData)
{
}

Resource& Resource::operator=(const Resource& another)
{
    m_resourceData = another.m_resourceData;
    return *this;
}

Resource::Resource(const Resource& resource)
    : m_resourceData(resource.m_resourceData)
{
}

Resource::~Resource()
{
}

