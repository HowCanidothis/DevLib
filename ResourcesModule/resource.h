#ifndef RESOURCE_H
#define RESOURCE_H

#include <SharedModule/internal.hpp>

class ResourceData
{
public:
    using FInitializer = std::function<void* ()>;
    using FReleaser = std::function<void (void*)>;

    ResourceData(const FInitializer& initializer, const FReleaser& releaser);
    virtual ~ResourceData();

protected:
    template<class> friend class TResource;
    friend class Resource;
    virtual void lock() const {}
    virtual void unlock() const {}
    virtual bool isThreadSafe() const { return false; }

private:
    void* use();
    void release();
    void* get() const;

private:
    void* m_data;
    FInitializer m_initializer;
    FReleaser m_releaser;
    qint32 m_useCount;
};

class ResourceDataThreadSafe : public ResourceData
{
    using Super = ResourceData;
public:
    using Super::Super;

protected:
    void lock() const override { m_mutex.lock(); }
    void unlock() const override { m_mutex.unlock(); }
    bool isThreadSafe() const override { return true; }

private:
    mutable QMutex m_mutex;
};

class Resource
{
public:
    Resource()
        : m_resourceData(nullptr)
    {}
    Resource(ResourceData* resourceData);
    Resource(const Resource& resource);

    virtual ~Resource();

    template<class T>
    void GetAccess(const std::function<void (T& resourceData)>& handler)
    {
        Q_ASSERT(m_resourceData != nullptr);
        m_resourceData->lock();
        handler(*(T*)m_resourceData->get());
        m_resourceData->unlock();
    }

    template<class T>
    void GetAccess(const std::function<void (const T& resourceData)>& handler) const
    {
        Q_ASSERT(m_resourceData != nullptr);
        m_resourceData->lock();
        handler(*(T*)m_resourceData->get());
        m_resourceData->unlock();
    }

    Resource& operator=(const Resource& another);

    bool IsNull() const { return m_resourceData == nullptr; }
    bool operator==(void* ptr) const { return m_resourceData == ptr; }
    bool operator!=(void* ptr) const { return m_resourceData != ptr; }

protected:
    ResourceData* m_resourceData;
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
        Q_ASSERT(!m_resourceData->isThreadSafe());
        return *(T*)m_resourceData->get();
    }
};

#endif // RESOURCE_H
