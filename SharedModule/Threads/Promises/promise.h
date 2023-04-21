#ifndef PROMISE_H
#define PROMISE_H

#include <QFuture>
#include <QFutureWatcher>

#include <functional>
#include <atomic>
#include <set>
#include <mutex>
#include <condition_variable>

#include "SharedModule/dispatcher.h"

class PromiseData ATTACH_MEMORY_SPY(PromiseData)
{
public:
    using FCallback = std::function<void (qint8)>;
    PromiseData();
    ~PromiseData();

private:
    void resolve(qint8 value);
    void resolve(const std::function<qint8 ()>& handler);
    DispatcherConnection then(const FCallback& handler);
    void mute();

private:
    friend class Promise;
    qint8 m_result;
    std::atomic_bool m_isResolved;
    std::atomic_bool m_isCompleted;
    CommonDispatcher<qint8> onFinished;
    std::mutex m_mutex;
};

struct SafeCallData
{
    bool ObjectIsDead = false;
};

class SafeCall
{
public:
    SafeCall();
    ~SafeCall();

    FAction Wrap(const FAction& handler) const;

private:
    SharedPointer<SafeCallData> m_data;
};

class Promise
{
    SharedPointer<PromiseData> m_data;
public:

    Promise();

    PromiseData* GetData() const { return m_data.get(); }
    qint8 GetValue() const { return m_data->m_result; }
    bool IsResolved() const { return m_data->m_isCompleted; }
    DispatcherConnection Then(const typename PromiseData::FCallback& handler) const { return m_data->then(handler); }
    DispatcherConnection Then(const typename PromiseData::FCallback& handler, const class FutureResult& future) const;
    void Resolve(qint8 value) const {  m_data->resolve(value); }
    void Resolve(const std::function<qint8 ()>& handler) const {  m_data->resolve(handler); }
    void Mute() { m_data->mute(); }

    template<typename ... Args>
    static Promise OnFirstInvokeWithResult(CommonDispatcher<Args...>& dispatcher, const typename CommonDispatcher<Args...>::FCommonDispatcherActionWithResult& acceptHandler = [](Args...) { return true; })
    {
        Promise result;
        auto connections = ::make_shared<DispatcherConnectionsSafe>();
        dispatcher.Connect(CONNECTION_DEBUG_LOCATION, [result, connections, acceptHandler](Args... args){
            if(acceptHandler(args...)) {
                connections->clear();
                result.Resolve(true);
            }
        }).MakeSafe(*connections);
        return result;
    }

    Promise MoveToMain(const std::function<qint8 (qint8)>& handler);
    qint8 Wait();
    bool Wait(qint32 msecs);
};

using AsyncResult = Promise;

class AsyncError : public AsyncResult
{
public:
    AsyncError();
};

class AsyncSuccess : public AsyncResult
{
public:
    AsyncSuccess();
};

class FutureResultData ATTACH_MEMORY_SPY(FutureResultData)
{
    template<class T> friend class QtFutureWatcher;
    friend class FutureResult;
    std::atomic<qint8> m_result;
    std::atomic<int> m_promisesCounter;
    std::condition_variable m_conditional;
    std::mutex m_mutex;

    void ref();
    void deref();

    bool isFinished() const;

    qint8 getResult() const;
    void setResult(qint8 result);

    void addPromise(const AsyncResult& promise, const SharedPointer<FutureResultData>& self);
    void then(const std::function<void (qint8)>& action);

    void wait();

    Dispatcher onFinished;

public:
    FutureResultData();
    ~FutureResultData();
};

class FutureResult ATTACH_MEMORY_SPY(FutureResult)
{
    template<class T> friend class QtFutureWatcher;
    SharedPointer<FutureResultData> m_data;
public:
    FutureResult();

    bool IsFinished() const;
    qint8 GetResult() const;
    void SetResult(qint8 result) const;

    void operator+=(const Promise& promise) const;

    void Then(const std::function<void (qint8)>& action) const;
    void Wait() const;
    bool Wait(qint32 msecs) const;

    AsyncResult ToAsyncResult() const;
};

template<class T>
class QtFutureWatcher : public QFutureWatcher<T>
{
    using Super = QFutureWatcher<T>;
    QFuture<T> m_future;
    AsyncResult m_result;
public:
    QtFutureWatcher(const QFuture<T>& future, const AsyncResult& result)
        : m_future(future)
        , m_result(result)
    {
        Super::connect(this, &QtFutureWatcher<T>::finished, [this](){
            m_result.Resolve(true);
            this->deleteLater();
        });
        Super::setFuture(future);
    }
};

#endif // PROMISE_H
