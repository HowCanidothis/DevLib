#ifndef PROMISE_H
#define PROMISE_H

#include <functional>
#include <atomic>
#include <set>
#include <mutex>
#include <condition_variable>

#include <SharedModule/smartpointersadapters.h>

template<class T>
class PromiseData
{
    typedef std::function<void (const T&)> FCallback;
    typedef std::function<void ()> FOnError;

    template<class> friend class Promise;
    friend class FutureResult;
    friend class AsyncObject;
    FCallback PromiseCallback;
    std::atomic_bool IsCompleted;
    std::mutex m_mutex;
    T ResolvedValue;

    PromiseData()
        : IsCompleted(false)
    {}
};

template<class T>
class Promise
{
    friend class FutureResult;
    SharedPointer<PromiseData<T>> m_data;
public:
    Promise()
        : m_data(new PromiseData<T>())
    {}

    void Resolve(const T& value)
    {
        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        m_data->ResolvedValue = value;
        m_data->IsCompleted = true;
        if(m_data->PromiseCallback) {
            m_data->PromiseCallback(value);
        }
    }

    void Then(const typename PromiseData<T>::FCallback& then)
    {
        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        Q_ASSERT(m_data->PromiseCallback == nullptr); // Only one "Then task" is supported
        if(m_data->IsCompleted) {
            then(m_data->ResolvedValue);
        } else {
            m_data->PromiseCallback = then;
        }
    }

    void Mute()
    {
        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        m_data->PromiseCallback = nullptr;
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

class FutureResult
{
    std::atomic<bool> m_result;
    std::atomic<int> m_promisesCounter;
    std::condition_variable m_conditional;
    std::mutex m_mutex;
#ifndef QT_NO_DEBUG
    std::set<PromiseData<bool>*> m_registeredPromises;
#endif
public:
    FutureResult()
        : m_result(false)
        , m_promisesCounter(0)
    {}

    FutureResult& operator+=(AsyncResult promise)
    {
        if(promise.m_data->IsCompleted)
        {
            if(!promise.m_data->ResolvedValue) {
                m_result = false;
            }
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_promisesCounter++;
#ifndef QT_NO_DEBUG
            auto data = promise.m_data.get();
            Q_ASSERT(m_registeredPromises.find(data) == m_registeredPromises.end());
            m_registeredPromises.insert(data);
#endif
        }
        promise.Then([this, promise](bool result){
            if(!result) {
                m_result = false;
            }
            *this -= promise;
        });

        return *this;
    }

    FutureResult& operator-=(const AsyncResult& promise)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_promisesCounter--;
#ifndef QT_NO_DEBUG
        auto data = promise.m_data.get();
        Q_ASSERT(m_registeredPromises.find(data) != m_registeredPromises.end());
        m_registeredPromises.erase(data);
#else
        Q_UNUSED(promise);
#endif
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
};

#endif // PROMISE_H
