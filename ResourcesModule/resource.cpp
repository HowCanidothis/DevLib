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

ResourceDataCapture ResourceDataThreadSafe::Capture()
{
    ResourceDataCapture result;

    if (auto activeWatcher = m_watcher.lock()) {
        result.Capture = activeWatcher;
        result.Value = m_data;
        result.m_lock = m_lock;
        result.m_unlock = m_unlock;
        return result;
    }

    QMutexLocker initLocker(&m_mutex);

    if (auto activeWatcher = m_watcher.lock()) {
        result.Capture = activeWatcher;
        result.Value = m_data;
        result.m_lock = m_lock;
        result.m_unlock = m_unlock;
    } else {
        auto mutex = ::make_shared<QMutex>();
        result.m_lock = m_lock = [mutex] { mutex->lock(); };
        result.m_unlock = m_unlock = [mutex] { mutex->unlock(); };
        auto* data = m_data = result.Value = m_onCaptured();
        auto releaser = m_onReleased;
        m_watcher = result.Capture = ::make_shared<SmartPointerWatcher>([data, releaser]{
            releaser(data);
        });
    }

    return result;
}

Resource::~Resource()
{
}

