#ifndef PROMISE_H
#define PROMISE_H

#include <QFuture>
#include <QFutureWatcher>
#include <QWaitCondition>

#include <functional>
#include <atomic>
#include <set>

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
    template<class ... Connections>
    void makeSafe(Connections&... connections)
    {
        Q_ASSERT(m_connection == nullptr);
        auto connection = DispatcherConnection([this]{
            resolve(false);
        }).MakeSafe(connections...);
        m_connection = ::make_shared<DispatcherConnectionSafe>(DispatcherConnection([connection]{
            connection->Disconnect();
        }));
    }

private:
    friend class Promise;
    template<class T> friend class PromisedValue;
    qint8 m_result;
    std::atomic_bool m_isResolved;
    std::atomic_bool m_isCompleted;
    SharedPointer<QMutex> m_mutex;
    CommonDispatcher<qint8> onFinished;
    DispatcherConnectionSafePtr m_connection;
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

template<class T>
class PromisedValue
{
    SP<T> m_data;
    SharedPointer<PromiseData> m_promiseData;

public:
    PromisedValue()
        : m_data(::make_shared<T>())
        , m_promiseData(::make_shared<PromiseData>())
    {
    }

    PromisedValue(const T& v)
        : m_data(::make_shared<T>(v))
        , m_promiseData(::make_shared<PromiseData>())
    {
    }

    void Fail() const { m_promiseData->resolve(false); }
    template<class Ret, typename FFunctor>
    PromisedValue<Ret> Then(const FFunctor& handler)
    {
        PromisedValue<Ret> result;
        auto d = m_data;
        m_promiseData->then([result, d, handler](qint8 ok){
            if(!ok) {
                result.Fail();
                return;
            }
            handler(*d, result).Then([result](qint8) {
                result.Fail(); // If not resolved early, will fail.
            });
        });
        return result;
    }

    template<typename FFunctor>
    PromisedValue<T> Then(const FFunctor& handler)
    {
        auto d = m_data;
        m_promiseData->then([d, handler](qint8 ok){
            if(!ok) {
                return;
            }
            handler(*d);
        });
        return *this;
    }

    void OnFailed(const FAction& action)
    {
        m_promiseData->then([action](qint8 ok){
            if(!ok) {
                action();
            }
        });
    }

    const PromisedValue& operator=(const T& v) const {
        m_promiseData->resolve([this, v]{
            *m_data = v;
            return true;
        });
        return *this;
    }
    operator const T&() const { return *m_data; }

private:
//    void link(const PromisedValue<T>& other) const {
//        auto self = *this;
//        other.m_promiseData->then([self, other](qint8 ok) {
//            if (ok) self = *other.m_data; // Assignment operator resolves self
//            else self.Fail();
//        });
//    }
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
    template<class Ret>
    PromisedValue<Ret> Then(const std::function<Ret ()>& handler)
    {
        PromisedValue<Ret> result;
        Then([handler, result](qint8 ok) {
            if(!ok) {
                result.Fail();
                return;
            }
            result = handler();
        });
        return result;
    }

    void Resolve(qint8 value) const {  m_data->resolve(value); }
    void Resolve(const std::function<qint8 ()>& handler) const {  m_data->resolve(handler); }
    void Mute() { m_data->mute(); }

    template<typename ... Args>
    static Promise OnFirstInvokeWithResult(const char* location, CommonDispatcher<Args...>& dispatcher, const typename CommonDispatcher<Args...>::FCommonDispatcherActionWithResult& acceptHandler = [](Args...) { return true; })
    {
        Promise result;
        auto connections = ::make_shared<DispatcherConnectionsSafe>();
        dispatcher.Connect(location, [result, connections, acceptHandler](Args... args){
            if(acceptHandler(args...)) {
                connections->clear();
                result.Resolve(true);
            }
        }).MakeSafe(*connections);
        return result;
    }

    template<typename ... Connections>
    void MakeSafe(Connections&... connections)
    {
        m_data->makeSafe(connections...);
    }
    Promise MoveToMain(const std::function<qint8 (qint8)>& handler);
    qint8 Wait();
    bool Wait(qint32 msecs);
};

using AsyncResult = Promise;

class AsyncSafeResult
{
public:
    ~AsyncSafeResult();

    void Release();
    AsyncSafeResult& operator=(const AsyncResult& res);

private:
    AsyncResult m_current;
};

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
    QWaitCondition m_conditional;
    SharedPointer<QMutex> m_mutex;
    Dispatcher onFinished;

    void ref();
    void deref();

    bool isFinished() const;

    qint8 getResult() const;
    void setResult(qint8 result);

    void addPromise(const AsyncResult& promise, const SharedPointer<FutureResultData>& self);
    void then(const std::function<void (qint8)>& action);

    void wait();

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
