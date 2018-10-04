#ifndef RESOURCE_H
#define RESOURCE_H

#include "resourcedata.h"

template<typename T>
class Resource
{
    ResourceDataBase* _data;
    Resource() {}
public:
    Resource(ResourceDataBase* data)
        : _data(data)
    {
        _data->InitResourceIfNeed();
    }

    virtual ~Resource()
    {
        _data->FreeResourceIfNeed<T>();
    }

    ResourceDataGuard<T> Data() { return ResourceDataGuard<T>(_data); }
    const ResourceDataGuard<T> Data() const { return ResourceDataGuard<T>(_data); }
    bool IsNull() const { return  _data->IsNull(); }
private:
    Resource& operator=(const Resource& other) = delete;
};

#endif // RESOURCE_H
