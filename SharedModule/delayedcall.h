#ifndef DELAYEDCALL_H
#define DELAYEDCALL_H

#include "SharedModule/Threads/Promises/promise.h"
#include "SharedModule/dispatcher.h"
#include "SharedModule/Threads/threadsbase.h"

class DelayedCallObject
{
public:
    DelayedCallObject(qint32 delayMsecs = 0, const ThreadHandlerNoThreadCheck& handler = ThreadHandlerNoCheckMainLowPriority);

    ~DelayedCallObject()
    {
        OnDeleted();
    }

    AsyncResult Call(const char* connectionInfo, const FAction& action);
    FAction Wrap(const char* connectionInfo, const FAction& action, const FAction& prepare = []{}) {
        return [this, action, prepare, connectionInfo]{ prepare(); Call(connectionInfo, action); };
    }

    Dispatcher OnDeleted;

    qint32 GetId() const { return m_id; }

private:
    static qint32 generateId();

    friend class DelayedCallManager;
    ThreadHandlerNoThreadCheck m_threadHandler;
    qint32 m_delay;
    qint32 m_id;
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
    static AsyncResult CallDelayed(const char* connectionInfo, DelayedCallObject* object, const FAction& action);

private:
    static QMutex* mutex();
    static QHash<qint32, DelayedCallPtr>& cachedCalls();
};

template<typename ... Args>
class DelayedCallDispatchersCommutator : public CommonDispatcher<Args...>
{
    using Super = CommonDispatcher<Args...>;
public:
    DelayedCallDispatchersCommutator(qint32 msecs = 0, const ThreadHandlerNoThreadCheck& threadHandler = ThreadHandlerNoCheckMainLowPriority)
        : m_delayedCallObject(msecs, threadHandler)
    {

    }

    void Invoke(Args... args) const override
    {
        m_delayedCallObject.Call(CONNECTION_DEBUG_LOCATION, [this, args...]{
            Super::Invoke(args...);
        });
    }

    void InvokeDirect() const
    {
        Super::Invoke();
    }

private:
    mutable DelayedCallObject m_delayedCallObject;
};

using DispatchersCommutator = DelayedCallDispatchersCommutator<>;

inline SharedPointer<DelayedCallObject> DelayedCallObjectCreate(qint32 msecs = 0, const ThreadHandlerNoThreadCheck& threadHandler = ThreadHandlerNoCheckMainLowPriority)
{
    return ::make_shared<DelayedCallObject>(msecs, threadHandler);
}

#endif // DELAYEDCALL_H
