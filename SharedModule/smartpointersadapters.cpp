#include "smartpointersadapters.h"

SmartPointer::SmartPointer(const std::function<void ()>& onCaptured, const std::function<void ()>& onReleased)
    : m_onCaptured(onCaptured)
    , m_onReleased(onReleased)
{}

SmartPointer::~SmartPointer()
{
    if(!m_watcher.expired()) {
        m_watcher.lock()->disconnect();
    }
}

SmartPointerWatcherPtr SmartPointer::Capture()
{
    SmartPointerWatcherPtr result;
    if(m_watcher.expired()) {
        m_onCaptured();
        result = ::make_shared<SmartPointerWatcher>(m_onReleased);
        m_watcher = result;
    } else {
        result = m_watcher.lock();
    }

    return result;
}

SmartPointerWatcher::SmartPointerWatcher(const std::function<void ()>& disconnector)
    : m_disconnector(disconnector)
{}

SmartPointerWatcher::~SmartPointerWatcher()
{
    if(m_disconnector != nullptr) {
        m_disconnector();
    }
}

void SmartPointerWatcher::disconnect()
{
    m_disconnector = nullptr;
}

SmartPointerValue::SmartPointerValue(const FInitializer& initializer, const FReleaser& releaser)
    : m_data(nullptr)
    , m_onCaptured(initializer)
    , m_onReleased(releaser)
{

}
SmartPointerValue::~SmartPointerValue()
{
}

SmartPointerValueCaptured SmartPointerValue::Capture()
{
    SmartPointerValueCaptured result;
    if(m_watcher.expired()) {
        auto* data = m_data = result.Value = m_onCaptured();
        auto releaser = m_onReleased;
        m_watcher = result.Capture = ::make_shared<SmartPointerWatcher>([data, releaser]{
            releaser(data);
        });
    } else {
        result.Capture = m_watcher.lock();
        result.Value = m_data;
    }

    return result;
}
