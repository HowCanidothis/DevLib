#include "resource.h"

ResourceBase::ResourceBase(const ResourceBase::LoadFunction& loader)
    : _data((void**)malloc(sizeof(size_t)))
    , _counter(new qint32)
    , _loader(loader)
{
    *_data = nullptr;
    *_counter = 0;
}

void ResourceBase::load() const{
    Q_ASSERT(*_data == nullptr);
    *_data = _loader();
}



