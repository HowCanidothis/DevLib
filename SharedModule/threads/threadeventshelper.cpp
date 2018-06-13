#include "threadeventshelper.h"
#include <QMutexLocker>

void ThreadEvent::call()
{
    _handler();
}

ThreadEventsContainer::ThreadEventsContainer()
{

}

void ThreadEventsContainer::Asynch(ThreadEvent::FEventHandler handler)
{
    QMutexLocker locker(&_eventsMutex);
    _events.Push(new ThreadEvent(handler));
}

void ThreadEventsContainer::ProcessEvents()
{
    QMutexLocker locker(&_eventsMutex);
    _eventsNotified = false;
    while(!_eventsNotified) { // from spurious wakeups
        _eventsProcessed.wait(&_eventsMutex);
    }
}

void ThreadEventsContainer::CallEvents()
{
    QMutexLocker locker(&_eventsMutex);
    for(ThreadEvent* event : _events) {
        event->call();
    }
    _events.Clear();
    _eventsNotified = true;
    _eventsProcessed.wakeAll();
}
