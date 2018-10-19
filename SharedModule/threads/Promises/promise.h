#ifndef PROMISE_H
#define PROMISE_H

#include <functional>
#include <SharedModule/smartpointersadapters.h>
#include <atomic>

template<class T>
class PromiseData
{
    typedef std::function<void (const T&)> FCallback;
    typedef std::function<void ()> FOnError;

    template<class T> friend class Promise;
    FCallback PromiseCallback;
    FOnError ErrorCallback;
    std::atomic_bool IsResolved;
    std::atomic_bool IsRejected;
    T ResolvedValue;

    PromiseData()
        : IsResolved(false)
        , IsRejected(false)
    {}
};

template<class T>
class Promise
{
    SharedPointer<PromiseData<T>> m_data;
public:
    Promise()
        : m_data(new PromiseData<T>())
    {}

    void Resolve(const T& value)
    {
        if(m_data->PromiseCallback) {
            m_data->PromiseCallback(value);
        } else {
            m_data->IsResolved = true;
            m_data->ResolvedValue = value;
        }
    }

    void Then(const typename PromiseData<T>::FCallback& then)
    {
        if(m_data->IsResolved) {
            then(m_data->ResolvedValue);
        }
        m_data->PromiseCallback = then;
    }

    Promise<T>& Catch(const typename PromiseData<T>::FOnError& catchHandler)
    {
        if(m_data->IsRejected) {
            catchHandler();
        }
        m_data->ErrorCallback = catchHandler;
        return *this;
    }

    void Reject()
    {
        if(m_data->ErrorCallback) {
            m_data->ErrorCallback();
        } else {
            m_data->IsRejected = true;
        }
    }
};

typedef Promise<bool> AsyncResult;

#endif // PROMISE_H
