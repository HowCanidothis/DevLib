#ifndef DELAYEDCALL_H
#define DELAYEDCALL_H

#include "SharedModule/Threads/Promises/promise.h"
#include "SharedModule/dispatcher.h"

class DelayedCallObject
{
public:
    DelayedCallObject(const ThreadHandlerNoThreadCheck& handler = ThreadHandlerNoCheckMainLowPriority)
        : m_threadHandler(handler)
    {}

    ~DelayedCallObject()
    {
        OnDeleted();
    }

    AsyncResult Call(const FAction& action);

    Dispatcher OnDeleted;

private:
    ThreadHandlerNoThreadCheck m_threadHandler;
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
    static AsyncResult CallDelayed(DelayedCallObject* object, const FAction& action, const ThreadHandlerNoThreadCheck& handler);
    static AsyncResult CallDelayedMain(DelayedCallObject* object, const FAction& action)
    {
        return CallDelayed(object, action, ThreadHandlerNoCheckMainLowPriority);
    }

private:
    static QMutex* mutex();
    static QHash<void*, DelayedCallPtr>& cachedCalls();
};

class DelayedCallDispatchersCommutator : public Dispatcher
{
public:
    DelayedCallDispatchersCommutator();

    // NOTE. It's eternal connection, non permanent connections will be added further if it becomes needed
    void Subscribe(const ThreadHandlerNoThreadCheck& threadHandler, const QVector<Dispatcher*>& dispatchers);
    void SubscribeMain(const QVector<Dispatcher*>& dispatchers) { Subscribe(ThreadHandlerNoCheckMainLowPriority, dispatchers); }

private:
    DelayedCallObject m_delayedCallObject;
};

inline AsyncResult DelayedCallObject::Call(const FAction& action)
{
    return DelayedCallManager::CallDelayed(this, action, m_threadHandler);
}

using DispatchersCommutator = DelayedCallDispatchersCommutator;

#endif // DELAYEDCALL_H
