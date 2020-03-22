#include "threadeventshelper.h"
#include <QMutexLocker>

#include "SharedModule/internal.hpp"

ThreadEvent::ThreadEvent(ThreadEvent::FEventHandler handler, const AsyncResult& result)
    : m_handler(handler)
    , m_result(result)
{}

void ThreadEvent::call()
{
    try {
        m_handler();
        m_result.Resolve(true);
    } catch (...) {
        m_result.Resolve(false);
    }
}

TagThreadEvent::TagThreadEvent(TagThreadEvent::TagsCache* tagsCache, const Name& tag, ThreadEvent::FEventHandler handler, const AsyncResult& result)
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
    try {
        m_handler();
        m_result.Resolve(true);
    } catch (...) {
        m_result.Resolve(false);
    }
}

ThreadEventsContainer::ThreadEventsContainer()
    : m_isPaused(false)
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

AsyncResult ThreadEventsContainer::Asynch(const Name& tag, ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&m_eventsMutex);
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

    if(m_events.empty()) {
        Asynch([]{});
    }

    while (!m_events.empty() && m_eventsMutex.tryLock()) {
        m_eventsMutex.unlock();
    }
}

AsyncResult ThreadEventsContainer::Asynch(ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&m_eventsMutex);
    AsyncResult result;
    m_events.push(new ThreadEvent(handler, result));
    return result;
}

void ThreadEventsContainer::clearEvents()
{
    m_interupted = true;
    ProcessEvents();
    QMutexLocker locker(&m_eventsMutex);
    m_events = std::queue<ThreadEvent*>();
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
