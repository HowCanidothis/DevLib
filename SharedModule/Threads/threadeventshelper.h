#ifndef THREADEVENTSHELPER_H
#define THREADEVENTSHELPER_H

#include <QWaitCondition>
#include <QMutex>
#include <QHash>
#include <queue>
#include <functional>
#include "SharedModule/name.h"
#include "SharedModule/Threads/Promises/promise.h"
#include "SharedModule/Threads/threads_declarations.h"

class ThreadEvent
{
public:
    ThreadEvent(const FAction& handler, const AsyncResult& result);
    virtual ~ThreadEvent() {}

protected:
    friend class ThreadEventsContainer;
    FAction m_handler;
    AsyncResult m_result;

    virtual void removeTag() {}
    virtual void call();
};

class TagThreadEvent : public ThreadEvent
{
public:
    typedef QHash<Name,ThreadEvent*> TagsCache;
    TagThreadEvent(TagsCache* tagsCache, const Name& tag, const FAction& handler, const AsyncResult& result);

protected:
    virtual void removeTag() Q_DECL_OVERRIDE;
    virtual void call() Q_DECL_OVERRIDE;

private:
    Name m_tag;
    TagsCache* m_tagsCache;
};

class _Export ThreadEventsContainer
{
public:
    typedef std::function<void ()> FOnPause;
    ThreadEventsContainer();
    virtual ~ThreadEventsContainer() = default;

    void Pause(const FOnPause& onPause);
    void Continue();

    AsyncResult Asynch(const Name& tag, const FAction& handler);
    AsyncResult Asynch(const FAction& handler);
    virtual void ProcessEvents();

    ThreadHandler CreateThreadHandler();

protected:
    void callEvents();
    void callPauseableEvents();
    void clearEvents();

private:
    std::queue<ThreadEvent*> m_events;
    QWaitCondition m_eventsProcessed;
    QWaitCondition m_eventsPaused;
    QMutex m_eventsMutex;
    std::atomic_bool m_eventsNotified;
    std::atomic_bool m_isPaused;
    std::atomic_bool m_interupted;
    FOnPause m_onPause;

    QHash<Name,ThreadEvent*> m_tagEventsMap;
};

#endif // THREADEVENTSHELPER_H
