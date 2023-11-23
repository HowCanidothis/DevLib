#include "threadeventshelper.h"
#include <QMutexLocker>

#include "SharedModule/internal.hpp"

ThreadEvent::ThreadEvent(const FAction& handler, const AsyncResult& result)
    : m_handler(handler)
    , m_result(result)
{}

void ThreadEvent::call()
{
    m_result.Resolve([&]{
        try
        {
            m_handler();
            return true;
        }
        catch (...)
        {
            return false;
        }
    });
}

ThreadHandler ThreadEventsContainer::CreateThreadHandler()
{
    return [this](const FAction& action) -> AsyncResult { return Asynch(action); };
}

TagThreadEvent::TagThreadEvent(TagThreadEvent::TagsCache* tagsCache, const Name& tag, const FAction& handler, const AsyncResult& result)
    : ThreadEvent(handler, result)
    , m_tag(tag)
    , m_tagsCache(tagsCache)
{
    Q_ASSERT(!tagsCache->contains(tag));
    tagsCache->insert(tag, this);
}

void TagThreadEvent::removeTag()
{
    m_tagsCache->remove(m_tag);
}

void TagThreadEvent::call()
{
    ThreadEvent::call();
}

ThreadEventsContainer::ThreadEventsContainer()
    : m_isPaused(false)
    , m_interupted(false)
    , m_disabled(false)
{

}

void ThreadEventsContainer::Continue()
{
    if(!m_isPaused) {
        return;
    }
    m_isPaused = false;
    m_eventsPaused.wakeAll();
}

void ThreadEventsContainer::Disable()
{
    {
        QMutexLocker locker(&m_eventsMutex);
        m_disabled = true;
    }
    clearEvents();
}

AsyncResult ThreadEventsContainer::Asynch(const Name& tag, const FAction& handler)
{
    QMutexLocker locker(&m_eventsMutex);
    if(m_disabled) {
        return AsyncError();
    }

    AsyncResult result;

    auto find = m_tagEventsMap.find(tag);
    if(find == m_tagEventsMap.end()) {
        auto tagEvent = new TagThreadEvent(&m_tagEventsMap, tag, handler, result);
        m_events.push(tagEvent);
    } else {
        find.value()->m_handler = handler;
        result = find.value()->m_result;
    }
    return result;
}

void ThreadEventsContainer::Pause(const FOnPause& onPause)
{
    if(m_isPaused) {
        return;
    }
    m_onPause = onPause;
    m_isPaused = true;

    /*if(m_events.empty()) {
        Asynch([]{});
    }

    while (!m_events.empty() && m_eventsMutex.tryLock()) {
        m_eventsMutex.unlock();
    }*/
}

AsyncResult ThreadEventsContainer::Asynch(const FAction& handler)
{
    QMutexLocker locker(&m_eventsMutex);
    if(m_disabled) {
        return AsyncError();
    }
    AsyncResult result;
    m_events.push(new ThreadEvent(handler, result));
    return result;
}

void ThreadEventsContainer::clearEvents()
{
    m_interupted = true;
    QMutexLocker locker(&m_eventsMutex);
    while(!m_events.empty()) {
        delete m_events.front();
        m_events.pop();
    }
    m_interupted = false;
}

void ThreadEventsContainer::ProcessEvents()
{
    QMutexLocker locker(&m_eventsMutex);
    while(!m_interupted && !m_events.empty()) { // from spurious wakeups
        m_eventsProcessed.wait(&m_eventsMutex);
    }
}

void ThreadEventsContainer::callEvents()
{
    Q_ASSERT(!m_disabled);
    while(!m_interupted && !m_events.empty()) {
        ScopedPointer<ThreadEvent> event;
        {
            QMutexLocker locker(&m_eventsMutex);
            event = m_events.front();
            m_events.pop();
            event->removeTag();
        }
        event->call();
    }

    m_eventsProcessed.wakeAll();
}

void ThreadEventsContainer::callPauseableEvents()
{
    Q_ASSERT(!m_disabled);
    while(!m_interupted && !m_events.empty()) {
        ScopedPointer<ThreadEvent> event;
        {
            if(m_isPaused) {
                m_onPause();
            }
            QMutexLocker locker(&m_eventsMutex);
            event = m_events.front();
            m_events.pop();
            while(m_isPaused) {
                m_eventsPaused.wait(&m_eventsMutex);
            }
            event->removeTag();
        }
        event->call();
    }

    m_eventsProcessed.wakeAll();
}
