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
class PromiseData ATTACH_MEMORY_SPY(PromiseData<T>)
{
    typedef std::function<void (const T&)> FCallback;

    template<class> friend class Promise;
    friend class FutureResult;
    friend class AsyncObject;
    CommonDispatcher<const T&> PromiseCallback;
    std::atomic_bool IsCompleted;
    std::mutex m_mutex;
    T ResolvedValue;

public:
    PromiseData()
        : IsCompleted(false)
    {}

    ~PromiseData()
    {

    }
};

template<class T>
class Promise
{
    friend class FutureResult;
    SharedPointer<PromiseData<T>> m_data;
public:
    Promise()
        : m_data(::make_shared<PromiseData<T>>())
    {}

    void Resolve(const T& value)
    {
        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        m_data->ResolvedValue = value;
        m_data->IsCompleted = true;
        m_data->PromiseCallback.Invoke(value);
        m_data->PromiseCallback -= m_data.get();
    }

    void Then(const typename PromiseData<T>::FCallback& then)
    {
        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        if(m_data->IsCompleted) {
            then(m_data->ResolvedValue);
        } else {
            m_data->PromiseCallback.Connect(m_data.get(), then);
        }
    }

    void Mute()
    {
        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        m_data->PromiseCallback -= m_data.get();
    }
};

typedef Promise<bool> AsyncResult;

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

class FutureResult ATTACH_MEMORY_SPY(FutureResult)
{
    std::atomic<bool> m_result;
    std::atomic<int> m_promisesCounter;
    std::condition_variable m_conditional;
    std::mutex m_mutex;
public:
    FutureResult()
        : m_result(true)
        , m_promisesCounter(0)
    {}

    bool GetResult() const { return m_result; }

    FutureResult& operator+=(AsyncResult promise)
    {
        if(promise.m_data->IsCompleted)
        {
            if(!promise.m_data->ResolvedValue) {
                m_result = false;
            }
            return *this;
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_promisesCounter++;
        }
        promise.Then([this, promise](const bool& result){
            if(!result) {
                m_result = false;
            }
            *this -= promise;
        });

        return *this;
    }

    template<class T>
    FutureResult& operator+=(Promise<T> promise)
    {
        if(promise.m_data->IsCompleted)
        {
            if(!promise.m_data->ResolvedValue) {
                m_result = false;
            }
            return *this;
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_promisesCounter++;
        }
        promise.Then([this, promise](const T&){
            *this -= promise;
        });

        return *this;
    }

    template<class T>
    FutureResult& operator-=(const Promise<T>&)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_promisesCounter--;
        m_conditional.notify_one();

        return *this;
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_promisesCounter != 0) {
            m_conditional.wait(lock);
        }
    }

    void Wait(Interruptor interuptor)
    {
        interuptor.OnInterrupted += {this, [this]{
            m_promisesCounter = 0;
            m_conditional.notify_all();
        }};
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_promisesCounter > 0) {
            m_conditional.wait(lock);
        }
        interuptor.OnInterrupted -= this;
    }
};

#endif // PROMISE_H
