#include "threadeventshelper.h"
#include <QMutexLocker>

#include "SharedModule/internal.hpp"

ThreadEvent::ThreadEvent(ThreadEvent::FEventHandler handler)
    : _handler(handler)
{}

void ThreadEvent::call()
{
    _handler();
}

TagThreadEvent::TagThreadEvent(TagThreadEvent::TagsCache* tagsCache, const Name& tag, ThreadEvent::FEventHandler handler)
    : ThreadEvent(handler)
    , _tag(tag)
    , _tagsCache(tagsCache)
{
    Q_ASSERT(!tagsCache->contains(tag));
    tagsCache->insert(tag, this);
}

void TagThreadEvent::removeTag()
{
    _tagsCache->remove(_tag);
}

void TagThreadEvent::call()
{
    _handler();
}

ThreadEventsContainer::ThreadEventsContainer()
    : _isPaused(false)
{

}

void ThreadEventsContainer::Continue()
{
    if(!_isPaused) {
        return;
    }
    _isPaused = false;
    _eventsPaused.wakeAll();
}

void ThreadEventsContainer::Asynch(const Name& tag, ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&_eventsMutex);

    auto find = _tagEventsMap.find(tag);
    if(find == _tagEventsMap.end()) {
        auto tagEvent = new TagThreadEvent(&_tagEventsMap, tag, handler);
        _events.push(tagEvent);
    } else {
        find.value()->_handler = handler;
    }
}

void ThreadEventsContainer::Pause(const FOnPause& onPause)
{
    if(_isPaused) {
        return;
    }
    _onPause = onPause;
    _isPaused = true;

    if(_events.empty()) {
        Asynch([]{});
    }

    while (!_events.empty() && _eventsMutex.tryLock()) {
        _eventsMutex.unlock();
    }
}

void ThreadEventsContainer::Asynch(ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&_eventsMutex);
    _events.push(new ThreadEvent(handler));
}

void ThreadEventsContainer::ProcessEvents()
{
    QMutexLocker locker(&_eventsMutex);
    while(!_events.empty()) { // from spurious wakeups
        _eventsProcessed.wait(&_eventsMutex);
    }
}

void ThreadEventsContainer::callEvents()
{
    while(!_events.empty()) {
        ScopedPointer<ThreadEvent> event;
        {
            QMutexLocker locker(&_eventsMutex);
            event = _events.front();
            _events.pop();
            event->removeTag();
        }
        event->call();
    }

    _eventsProcessed.wakeAll();
}

void ThreadEventsContainer::callPauseableEvents()
{
    while(!_events.empty()) {
        ScopedPointer<ThreadEvent> event;
        {
            if(_isPaused) {
                _onPause();
            }
            QMutexLocker locker(&_eventsMutex);
            event = _events.front();
            _events.pop();
            while(_isPaused) {
                _eventsPaused.wait(&_eventsMutex);
            }
            event->removeTag();
        }
        event->call();
    }

    _eventsProcessed.wakeAll();
}
