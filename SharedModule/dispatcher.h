#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QHash>
#include <QMutex>

#include <functional>

#include "smartpointersadapters.h"
#include "stack.h"

template<typename ... Args>
class CommonDispatcher
{
public:
    using FCommonDispatcherAction = std::function<void (Args...)>;
    typedef void* Observer;
    struct ActionHandler
    {
        Observer Key;
        FCommonDispatcherAction Handler;
    };

    bool IsEmpty() const
    {
        return m_subscribes.isEmpty();
    }

    void Invoke(Args... args) const
    {
        QMutexLocker lock(&m_mutex);
        for(auto subscribe : m_subscribes)
        {
            subscribe(args...);
        }
    }

    void operator()(Args... args) const
    {
        Invoke(args...);
    }

    CommonDispatcher& operator+=(const ActionHandler& subscribeHandler)
    {
        QMutexLocker lock(&m_mutex);
        Q_ASSERT(!m_subscribes.contains(subscribeHandler.Key));
        m_subscribes.insert(subscribeHandler.Key, subscribeHandler.Handler);
        return *this;
    }

    CommonDispatcher& operator-=(Observer observer)
    {
        QMutexLocker lock(&m_mutex);
        m_subscribes.remove(observer);
        return *this;
    }

private:
    QHash<Observer, FCommonDispatcherAction> m_subscribes;
    mutable QMutex m_mutex;
};

class Dispatcher : public CommonDispatcher<>
{};

class DispatchersConnections : private Stack<Dispatcher*>
{
    using Super = Stack<Dispatcher*>;

    Dispatcher::Observer* m_observer;

    Q_DISABLE_COPY(DispatchersConnections);
#ifndef QT_NO_DEBUG
    QSet<Dispatcher*> m_dispatchersGuard;
#endif

public:
    DispatchersConnections(void* observer)
        : m_observer(reinterpret_cast<Dispatcher::Observer*>(observer))
    {}

    ~DispatchersConnections()
    {
        for(auto* dispatcher : *this) {
            *dispatcher -= m_observer;
        }
    }

    void Add(Dispatcher& dispatcher, const FAction& action)
    {
#ifndef QT_NO_DEBUG
        Q_ASSERT(!m_dispatchersGuard.contains(&dispatcher));
        m_dispatchersGuard.insert(&dispatcher);
#endif

        dispatcher += { m_observer, action };
        this->Append(&dispatcher);
    }

    void Clear()
    {
        for(auto* dispatcher : *this) {
            *dispatcher -= m_observer;
        }
        Super::Clear();
#ifndef QT_NO_DEBUG
        m_dispatchersGuard.clear();
#endif
    }
};

#endif // NOTIFICATION_H
