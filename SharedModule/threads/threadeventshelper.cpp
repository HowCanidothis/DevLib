#include "threadeventshelper.h"
#include <QMutexLocker>

#include "SharedModule/internal.hpp"

ThreadEvent::ThreadEvent(ThreadEvent::FEventHandler handler, const AsyncResult& result)
    : _handler(handler)
    , _result(result)
{}

void ThreadEvent::call()
{
    try {
        _handler();
        _result.Resolve(true);
    } catch (...) {
        _result.Resolve(false);
    }
}

TagThreadEvent::TagThreadEvent(TagThreadEvent::TagsCache* tagsCache, const Name& tag, ThreadEvent::FEventHandler handler, const AsyncResult& result)
    : ThreadEvent(handler, result)
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
    try {
        _handler();
        _result.Resolve(true);
    } catch (...) {
        _result.Resolve(false);
    }
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

AsyncResult ThreadEventsContainer::Asynch(const Name& tag, ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&_eventsMutex);
    AsyncResult result;

    auto find = _tagEventsMap.find(tag);
    if(find == _tagEventsMap.end()) {
        auto tagEvent = new TagThreadEvent(&_tagEventsMap, tag, handler, result);
        _events.push(tagEvent);
    } else {
        find.value()->_handler = handler;
        result = find.value()->_result;
    }
    return result;
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

AsyncResult ThreadEventsContainer::Asynch(ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&_eventsMutex);
    AsyncResult result;
    _events.push(new ThreadEvent(handler, result));
    return result;
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
