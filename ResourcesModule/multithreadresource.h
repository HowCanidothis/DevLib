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

    T* data() { return p_resource->data(); }
private:
    MultiThreadResource<T>* p_resource;
};

template<typename T>
class MultiThreadResource : public Resource<T>
{
    typedef Resource<T> Super;
public:
    using Resource::Resource;  

    MultiThreadResourceGuard<T> lock() { return MultiThreadResourceGuard<T>(this); }

private:
    QMutex mutex;

private:
    T* data() { return Super::data(); }
    void lockInternal() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    friend class MultiThreadResourceGuard<T>;
};

template<typename T>
MultiThreadResourceGuard<T>::MultiThreadResourceGuard(MultiThreadResource<T>* resource)
    : p_resource(resource)
{
    p_resource->lockInternal();
}

template<typename T>
MultiThreadResourceGuard<T>::~MultiThreadResourceGuard()
{
    p_resource->unlock();
}

#endif // MULTITHREADRESOURCE_H
