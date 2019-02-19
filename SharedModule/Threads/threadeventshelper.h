#ifndef THREADEVENTSHELPER_H
#define THREADEVENTSHELPER_H

#include <QWaitCondition>
#include <QMutex>
#include <QHash>
#include <queue>
#include <functional>
#include "SharedModule/name.h"
#include "SharedModule/threads/Promises/promise.h"

class ThreadEvent
{
public:
    typedef std::function<void()> FEventHandler;

    ThreadEvent(FEventHandler handler, const AsyncResult& result);
    virtual ~ThreadEvent() {}

protected:
    friend class ThreadEventsContainer;
    FEventHandler _handler;
    AsyncResult _result;

    virtual void removeTag() {}
    virtual void call();
};

class TagThreadEvent : public ThreadEvent
{
public:
    typedef QHash<Name,ThreadEvent*> TagsCache;
    TagThreadEvent(TagsCache* tagsCache, const Name& tag, FEventHandler handler, const AsyncResult& result);

protected:
    virtual void removeTag() Q_DECL_OVERRIDE;
    virtual void call() Q_DECL_OVERRIDE;

private:
    Name _tag;
    TagsCache* _tagsCache;
};

class _Export ThreadEventsContainer
{
public:
    typedef std::function<void ()> FOnPause;
    ThreadEventsContainer();
    virtual ~ThreadEventsContainer() = default;

    void Pause(const FOnPause& onPause);
    void Continue();

    AsyncResult Asynch(const Name& tag, ThreadEvent::FEventHandler handler);
    AsyncResult Asynch(ThreadEvent::FEventHandler handler);
    void ProcessEvents();

protected:
    void callEvents();
    void callPauseableEvents();

private:
    std::queue<ThreadEvent*> _events;
    QWaitCondition _eventsProcessed;
    QWaitCondition _eventsPaused;
    QMutex _eventsMutex;
    std::atomic_bool _eventsNotified;
    std::atomic_bool _isPaused;
    FOnPause _onPause;

    QHash<Name,ThreadEvent*> _tagEventsMap;
};

#endif // THREADEVENTSHELPER_H
