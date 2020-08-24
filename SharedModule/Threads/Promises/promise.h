#ifndef PROMISE_H
#define PROMISE_H

#include <functional>
#include <atomic>
#include <set>
#include <mutex>
#include <condition_variable>

#include "SharedModule/smartpointersadapters.h"
#include "SharedModule/shared_decl.h"
#include "SharedModule/dispatcher.h"
#include "SharedModule/interruptor.h"

template<class T>
class PromiseData
{
public:
    using FCallback = std::function<void (const T& )>;
    PromiseData()
        : m_isResolved(false)
    {}

private:
    template<class T2> friend class Promise;
    T m_result;
    std::atomic_bool m_isResolved;
    CommonDispatcher<bool> onFinished;
    std::mutex m_mutex;

    void resolve(bool value)
    {
        if(!m_isResolved) {
            m_isResolved = true;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_result = value;
            }
            onFinished(value);
            onFinished -= this;
        }
    }

    void then(const FCallback& handler)
    {
        if(m_isResolved) {
            handler(m_result);
        } else {
            onFinished.Connect(this, handler);
        }
    }

    void mute()
    {
         onFinished -= this;
    }
};

template<class T>
class Promise
{
    SharedPointer<PromiseData<T>> m_data;
public:

    Promise()
        : m_data(::make_shared<PromiseData<T>>())
    {}

    const T& GetValue() const { return m_data->m_result; }
    bool IsResolved() const { return m_data->m_isResolved; }
    void Then(const typename PromiseData<T>::FCallback& handler) { m_data->then(handler); }
    void Resolve(bool value) {  m_data->resolve(value); }
    void Mute() { m_data->mute(); }
};

using AsyncResult = Promise<bool>;

class AsyncError : public AsyncResult
{
public:
    AsyncError() {
        Resolve(false);
    }
};

class AsyncObject
{
public:
    AsyncObject();
    ~AsyncObject();

    AsyncResult Async(const FAction& action, const PromiseData<bool>::FCallback& onDone);

private:
    AsyncResult m_result;
};

class FutureResultData
{
    template<class T> friend class QtFutureWatcher;
    friend class FutureResult;
    std::atomic<bool> m_result;
    std::atomic<int> m_promisesCounter;
    std::condition_variable m_conditional;
    std::mutex m_mutex;
    QVector<AsyncResult> m_keepedResults;

    void ref()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_promisesCounter++;
    }
    void deref()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_promisesCounter--;

        if(isFinished()) {
            m_conditional.notify_one();
            onFinished();
        }
    }

    bool isFinished() const { return m_promisesCounter == 0; }
    bool getResult() const { return m_result; }

    void operator+=(AsyncResult promise)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_keepedResults.append(promise); // Garanting that all results will be valid
        }
        m_promisesCounter++;
        promise.Then([this, promise](const bool& result){
            if(!result) {
                m_result = false;
            }
            *this -= promise;
        });

        return;
    }

    template<class T>
    void operator+=(Promise<T> promise)
    {
        ref();
        promise.Then([this, promise](const T&){
            *this -= promise;
        });
    }

    void operator-=(const AsyncResult&)
    {
        deref();
    }

    template<class T>
    void operator-=(const Promise<T>&)
    {
        deref();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(!isFinished()) {
            m_conditional.wait(lock);
        }
    }

    void wait(Interruptor interruptor)
    {
        interruptor.OnInterrupted += {this, [this]{
            m_promisesCounter = 0;
            m_conditional.notify_all();
        }};
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_promisesCounter > 0) {
            m_conditional.wait(lock);
        }
        interruptor.OnInterrupted -= this;
    }

    Dispatcher onFinished;

public:
    FutureResultData()
        : m_result(true)
        , m_promisesCounter(0)
    {}
};

class FutureResult ATTACH_MEMORY_SPY(FutureResult)
{
    template<class T> friend class QtFutureWatcher;
    SharedPointer<FutureResultData> m_data;
public:
    enum MemoryPolicy
    {
        MemoryPolicy_DeleteOnFinished
    };
    FutureResult()
        : m_data(::make_shared<FutureResultData>())
        , OnFinished(m_data->onFinished)
    {}
    FutureResult(MemoryPolicy)
        : FutureResult()
    {
        OnFinished += { this, [this] { delete this; } };
    }

    bool IsFinished() const { return m_data->isFinished(); }
    bool GetResult() const { return m_data->getResult(); }

    void operator-=(const AsyncResult& promise)
    {
        *m_data += promise;
    }

    template<class T>
    void operator+=(const Promise<T>& promise)
    {
        *m_data += promise;
    }

    template<class T>
    void operator-=(const Promise<T>& promise)
    {
        *m_data -= promise;
    }

    void Wait()
    {
        m_data->wait();
    }

    void Wait(Interruptor interruptor)
    {
        m_data->wait(interruptor);
    }

    Dispatcher& OnFinished;

};

#include <QFuture>
#include <QFutureWatcher>
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
