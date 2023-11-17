#ifndef DELAYEDCALL_H
#define DELAYEDCALL_H

#include "SharedModule/Threads/Promises/promise.h"
#include "SharedModule/dispatcher.h"
#include "SharedModule/Threads/threadsbase.h"

struct DelayedCallObjectParams
{
    DelayedCallObjectParams(qint32 delayMsecs = 0, const ThreadHandlerNoThreadCheck& handler = ThreadHandlerNoCheckMainLowPriority)
        : DelayMsecs(delayMsecs)
        , Handler(handler)
    {}

    qint32 DelayMsecs;
    ThreadHandlerNoThreadCheck Handler;
};

class DelayedCallObject
{
public:
    DelayedCallObject(const DelayedCallObjectParams& params = DelayedCallObjectParams());
    ~DelayedCallObject();

    AsyncResult Call(const char* connectionInfo, const FAction& action);
    FAction Wrap(const char* connectionInfo, const FAction& action, const FAction& prepare = []{});

    Dispatcher OnDeleted;

    qint32 GetId() const { return m_id; }

private:
    static qint32 generateId();

    friend class DelayedCallManager;
    DelayedCallObjectParams m_params;
    qint32 m_id;
};

using DelayedCallObjectPtr = SharedPointer<DelayedCallObject>;

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
    static AsyncResult CallDelayed(const char* connectionInfo, const Name& key, const FAction& action, const DelayedCallObjectParams& params = DelayedCallObjectParams());

    static void Terminate();

private:
    static QMutex* mutex();
    static QHash<qint32, DelayedCallPtr>& cachedCalls();
    static QHash<Name, DelayedCallObjectPtr>& namedObjects();
};

template<typename ... Args>
class DelayedCallDispatchersCommutator : public CommonDispatcher<Args...>
{
    using Super = CommonDispatcher<Args...>;
public:
    DelayedCallDispatchersCommutator(const DelayedCallObjectParams& params = DelayedCallObjectParams())
        : m_delayedCallObject(params)
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

template<typename ... Args>
class DelayedCallDispatchersCommutatorWithDirect : public DelayedCallDispatchersCommutator<Args...>
{
    using Super = DelayedCallDispatchersCommutator<Args...>;
public:
    using Super::Super;

    void Invoke(Args... args) const override
    {
        OnDirectChanged(args...);
        Super::Invoke(args...);
    }

    CommonDispatcher<Args...> OnDirectChanged;
};

using DispatchersCommutator = DelayedCallDispatchersCommutator<>;
using DispatchersCommutatorWithDirect = DelayedCallDispatchersCommutatorWithDirect<>;

inline DelayedCallObjectPtr DelayedCallObjectCreate(const DelayedCallObjectParams& params = DelayedCallObjectParams())
{
    return ::make_shared<DelayedCallObject>(params);
}

#endif // DELAYEDCALL_H
