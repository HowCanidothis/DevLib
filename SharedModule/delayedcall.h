#ifndef DELAYEDCALL_H
#define DELAYEDCALL_H

#include "SharedModule/Threads/Promises/promise.h"

class DelayedCallObject
{
public:
    ~DelayedCallObject()
    {
        OnDeleted();
    }

    Dispatcher OnDeleted;
};

class DelayedCall
{
public:
    DelayedCall(const FAction& action, QMutex* mutex, DelayedCallObject* object);

    void Call();
    void SetAction(const FAction &action);
    void SetResult(const AsyncResult& result);

    const AsyncResult& GetResult() const { return m_result; }

private:
    FAction m_action;
    DispatcherConnectionSafePtr m_connection;
    AsyncResult m_result;
    QMutex* m_mutex;
};

using DelayedCallPtr = SharedPointer<DelayedCall>;

class DelayedCallManager
{
public:
    static AsyncResult CallDelayed(DelayedCallObject* object, const FAction& action, const ThreadHandler& handler);
    static AsyncResult CallDelayedMain(DelayedCallObject* object, const FAction& action)
    {
        return CallDelayed(object, action, [](const FAction& action) -> AsyncResult { return ThreadsBase::DoMainWithResult(action, Qt::LowEventPriority); });
    }

private:
    static QMutex* mutex();
    static QHash<void*, DelayedCallPtr>& cachedCalls();
};

#endif // DELAYEDCALL_H
