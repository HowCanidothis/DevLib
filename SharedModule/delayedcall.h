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

    AsyncResult Call(const FAction& action);

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
    static AsyncResult CallDelayed(DelayedCallObject* object, const FAction& action);

private:
    static QMutex* mutex();
    static QHash<qint32, DelayedCallPtr>& cachedCalls();
};

template<class T, class T2> class LocalProperty;
template<class T> struct LocalPropertyOptional;

class DelayedCallDispatchersCommutator : public Dispatcher
{
public:
    DelayedCallDispatchersCommutator(qint32 msecs = 0, const ThreadHandlerNoThreadCheck& threadHandler = ThreadHandlerNoCheckMainLowPriority);

    // NOTE. It's eternal connection, non permanent connections will be added further if it becomes needed
    DispatcherConnections Subscribe(const QVector<CommonDispatcher<>*>& dispatchers);
    DispatcherConnection Subscribe(CommonDispatcher* dispatchers);

#ifdef PROPERTIES_LIB
    template<class T, class T2>
    DispatcherConnection Subscribe(LocalProperty<T, T2>& property);
    template<class T>
    DispatcherConnections Subscribe(LocalPropertyOptional<T>& property);
#endif

    void operator()() = delete;

    void InvokeDelayed();
    void InvokeDirect();

private:
    DelayedCallObject m_delayedCallObject;
};

inline AsyncResult DelayedCallObject::Call(const FAction& action)
{
    return DelayedCallManager::CallDelayed(this, action);
}

using DispatchersCommutator = DelayedCallDispatchersCommutator;

template<typename ... Args>
class DelayedCallCommonDispatcher : public CommonDispatcher<Args...>
{
    using Super = CommonDispatcher<Args...>;
public:
    DelayedCallCommonDispatcher(qint32 delayMsecs = 0, const ThreadHandlerNoThreadCheck& handler = ThreadHandlerNoCheckMainLowPriority)
        : m_delayedInvoke(delayMsecs, handler)
    {}

    void Invoke(Args... args) const override
    {
        m_delayedInvoke.Call([this, args...]{
            Super::Invoke(args...);
        });
    }

private:
    mutable DelayedCallObject m_delayedInvoke;
};

using DelayedCallDispatcher = DelayedCallCommonDispatcher<>;

#endif // DELAYEDCALL_H
