#ifndef MULTITHREADRESOURCE_H
#define MULTITHREADRESOURCE_H

#include <QMutex>

#include "resource.h"

template<typename T> class MultiThreadResource;

template<typename T>
class MultiThreadResourceGuard
{
public:
    MultiThreadResourceGuard(MultiThreadResource<T>* resource);
    ~MultiThreadResourceGuard();

    T* Data() { return _resource->Data(); }

private:
    MultiThreadResource<T>* _resource;
};

template<typename T>
class MultiThreadResource : public Resource<T>
{
    typedef Resource<T> Super;
public:
    using Resource::Resource;  

    MultiThreadResourceGuard<T> Lock() { return MultiThreadResourceGuard<T>(this); }

private:
    QMutex _mutex;

private:
    T* Data() { return Super::Data(); }
    void lockInternal() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

    friend class MultiThreadResourceGuard<T>;
};

template<typename T>
MultiThreadResourceGuard<T>::MultiThreadResourceGuard(MultiThreadResource<T>* resource)
    : _resource(resource)
{
    _resource->lockInternal();
}

template<typename T>
MultiThreadResourceGuard<T>::~MultiThreadResourceGuard()
{
    _resource->unlock();
}

#endif // MULTITHREADRESOURCE_H
