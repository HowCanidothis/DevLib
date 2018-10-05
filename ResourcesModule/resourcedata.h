#ifndef RESOURCEDATA_H
#define RESOURCEDATA_H

#include <Qt>
#include <QMutex>
#include <functional>

class ResourceDataBase
{
    typedef std::function<void*()> LoadFunction;

    void** _data;
    qint32* _counter;
    LoadFunction _loader;
public:
    ResourceDataBase(const LoadFunction& loader);

    bool IsNull() const { return  *_data == nullptr; } // TODO. it cans be unsafe

    void InitResourceIfNeed()
    {
        lock();
        (*_counter)++;
        if(*_data == nullptr) {
            load();
        }
        unlock();
    }

    template<class T> void FreeResourceIfNeed()
    {
        lock();
        if(--(*_counter) == 0) {
            delete (T*)*_data;
            *_data = nullptr;
        }
        unlock();
    }

private:
    template<class T> friend class ResourceDataGuard;

    virtual void lock() {}
    virtual void unlock() {}

    void load() const;
};

class ResourceDataMultiThread : public ResourceDataBase
{
    QMutex _mutex;
public:
    using ResourceDataBase::ResourceDataBase;

    virtual void lock() { _mutex.lock(); }
    virtual void unlock() { _mutex.unlock(); }
};

template<class T>
class ResourceDataGuard
{
    ResourceDataBase* _data;
public:
    ResourceDataGuard(ResourceDataBase* data)
        : _data(data)
    {
        _data->lock();
    }

    ~ResourceDataGuard()
    {
        _data->unlock();
    }

    void Set(const T& value)
    {
        *data() = value;
    }

    T& Change()
    {
        return *data();
    }

    const T& Get() const
    {
        return *data();
    }

private:
    T* data() const { return *(T**)(_data->_data); }
};


#endif // RESOURCEDATA_H
