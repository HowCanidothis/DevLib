#ifndef DELAYEDCALL_H
#define DELAYEDCALL_H

#include "SharedModule/Threads/Promises/promise.h"
#include "SharedModule/dispatcher.h"

class DelayedCallObject
{
public:
    DelayedCallObject(qint32 delayMsecs = 0, const ThreadHandlerNoThreadCheck& handler = ThreadHandlerNoCheckMainLowPriority)
        : m_threadHandler(handler)
        , m_delay(delayMsecs)
    {}

    ~DelayedCallObject()
    {
        OnDeleted();
    }

    AsyncResult Call(const FAction& action);

    Dispatcher OnDeleted;

private:
    friend class DelayedCallManager;
    ThreadHandlerNoThreadCheck m_threadHandler;
    qint32 m_delay;
};

class DelayedCall
{
public:
    DelayedCall(const FAction& action, QMutex* mutex, DelayedCallObject* object);

    virtual void Call();
    virtual AsyncResult Invoke(const ThreadHandlerNoThreadCheck& threadHandler, const FAction& delayedCall, qint32 delay);
    void SetAction(const FAction& action);
    void SetResult(const AsyncResult& result);

    const AsyncResult& GetResult() const { return m_result; }

private:
    FAction m_action;
    DispatcherConnectionSafePtr m_connection;
    DispatcherConnectionSafePtr m_resultConnection;
    AsyncResult m_result;
    QMutex* m_mutex;
};

class DelayedCallDelayOnCall : public DelayedCall
{
    using Super = DelayedCall;
public:
    DelayedCallDelayOnCall(const FAction& action, QMutex* mutex, DelayedCallObject* object);

    void Call() override;
    AsyncResult Invoke(const ThreadHandlerNoThreadCheck& threadHandler, const FAction& delayedCall, qint32 delay) override;

private:
    std::atomic_int m_counter;
};

using DelayedCallPtr = SharedPointer<DelayedCall>;

class DelayedCallManager
{
public:
    static AsyncResult CallDelayed(DelayedCallObject* object, const FAction& action);

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
    return DelayedCallManager::CallDelayed(this, action);
}

using DispatchersCommutator = DelayedCallDispatchersCommutator;

#endif // DELAYEDCALL_H
