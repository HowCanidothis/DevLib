#ifndef RESOURCE_H
#define RESOURCE_H

#include <SharedModule/internal.hpp>

struct ResourceDataCapture : private SmartPointerValueCaptured
{
    ResourceDataCapture()
    {
        Value = nullptr;
    }

    ResourceDataCapture& operator=(const SmartPointerValueCaptured& c)
    {
        Capture = c.Capture;
        Value = c.Value;
        return *this;
    }

private:
    bool isThreadSafe() const { return m_lock != nullptr; }
    void* get() const { return Value; }
    template<class T>
    T& get() const { return *static_cast<T*>(Value); }
    void lock() { if(m_lock) m_lock(); }
    void unlock() { if(m_unlock) m_unlock(); }

private:
    template<class> friend class TResource;
    friend class Resource;
    friend class ResourceDataThreadSafe;
    FAction m_lock, m_unlock;
};

class ResourceData : public SmartPointerValue
{
    using Super = SmartPointerValue;
public:
    using Super::Super;

    virtual ResourceDataCapture Capture()
    {
        ResourceDataCapture result;
        result = Super::Capture();
        return result;
    }
};

class ResourceDataThreadSafe : public ResourceData
{
    using Super = ResourceData;
public:
    using Super::Super;

    virtual ResourceDataCapture Capture()
    {
        ResourceDataCapture result;
        if(m_watcher.expired()) {
            auto mutex = ::make_shared<QMutex>();
            result.m_lock = m_lock = [mutex] { mutex->lock(); };
            result.m_unlock = m_unlock = [mutex] { mutex->unlock(); };
            auto* data = m_data = result.Value = m_onCaptured();
            auto releaser = m_onReleased;
            m_watcher = result.Capture = ::make_shared<SmartPointerWatcher>([data, releaser]{
                releaser(data);
            });
        } else {
            result.Capture = m_watcher.lock();
            result.Value = m_data;
            result.m_lock = m_lock;
            result.m_unlock = m_unlock;
        }

        return result;
    }

private:
    FAction m_lock;
    FAction m_unlock;
};

class Resource
{
public:
    Resource(){}
    Resource(const ResourceDataCapture& capture);
    Resource(const Resource& resource);

    virtual ~Resource();

    template<class T>
    void GetAccess(const std::function<void (T& resourceData)>& handler)
    {
        Q_ASSERT(m_resourceData.get() != nullptr);
        m_resourceData.lock();
        handler(m_resourceData.get<T>());
        m_resourceData.unlock();
    }

    template<class T>
    void GetAccess(const std::function<void (const T& resourceData)>& handler) const
    {
        Q_ASSERT(m_resourceData.get() != nullptr);
        m_resourceData.lock();
        handler(m_resourceData.get<T>());
        m_resourceData.unlock();
    }

    Resource& operator=(const Resource& another);

    bool operator==(void* ptr) const { return m_resourceData.get() == ptr; }
    bool operator!=(void* ptr) const { return m_resourceData.get() != ptr; }

protected:
    ResourceDataCapture m_resourceData;
};

template<class T>
class TResource : public Resource
{
    using Super = Resource;
public:
    using Super::Super;

    TResource& operator=(const T& value)
    {
        GetAccess([&](T& data){
            data = value;
        });
        return *this;
    }

    void GetAccess(const std::function<void (T& resourceData)>& handler)
    {
        Super::GetAccess<T>(handler);
    }

    void GetAccess(const std::function<void (const T& resourceData)>& handler) const
    {
        Super::GetAccess<T>(handler);
    }

    const T& Get() const
    {
        Q_ASSERT(!m_resourceData.isThreadSafe());
        return m_resourceData.get<T>();
    }
};

#endif // RESOURCE_H
