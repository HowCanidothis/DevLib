#ifndef RESOURCE_H
#define RESOURCE_H

#include <SharedModule/internal.hpp>

#include "resourcedata.h"

template<typename T>
class Resource
{
    ResourceDataBase* m_data;
    Resource() {}
public:
    Resource(ResourceDataBase* data)
        : m_data(data)
    {
        m_data->InitResourceIfNeeded();
    }

    virtual ~Resource()
    {
        m_data->FreeResourceIfNeeded<T>();
    }

    ResourceDataGuard<T> Data() { return ResourceDataGuard<T>(m_data); }
    const ResourceDataGuard<T> Data() const { return ResourceDataGuard<T>(m_data); }
    bool IsNull() const { return  m_data->IsNull(); }
private:
    Resource& operator=(const Resource& other) = delete;
};

template<class T> using ResourcePtr = SharedPointer<Resource<T>>;

#endif // RESOURCE_H
