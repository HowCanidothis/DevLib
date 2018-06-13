#ifndef THREADEVENTSHELPER_H
#define THREADEVENTSHELPER_H

#include <QWaitCondition>
#include <QMutex>

#include "SharedModule/stack.h"

class ThreadEvent
{
public:
    typedef std::function<void()> FEventHandler;

    ThreadEvent(FEventHandler  handler)
        : _handler(handler)
    {}
private:
    friend class ThreadEventsContainer;
    FEventHandler _handler;

    void call();
};

class ThreadEventsContainer
{
public:
    ThreadEventsContainer();

    void Asynch(ThreadEvent::FEventHandler handler);
    void ProcessEvents();

protected:
    void CallEvents();

private:
    StackPointers<ThreadEvent> _events;
    QWaitCondition _eventsProcessed;
    QMutex _eventsMutex;
    std::atomic_bool _eventsNotified;
};

#endif // THREADEVENTSHELPER_H
