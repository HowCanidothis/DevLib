#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QHash>
#include <QMutex>

#include <functional>

#include "smartpointersadapters.h"
#include "stack.h"

class Dispatcher
{
public:
    typedef void* Observer;
    struct ActionHandler
    {
        Observer Key;
        FAction Handler;
    };

    bool IsEmpty() const
    {
        return m_subscribes.isEmpty();
    }

    void Invoke() const
    {
        QMutexLocker lock(&m_mutex);
        for(auto subscribe : m_subscribes)
        {
            subscribe();
        }
    }

    void operator()() const
    {
        Invoke();
    }

    Dispatcher& operator+=(const ActionHandler& subscribeHandler)
    {
        QMutexLocker lock(&m_mutex);
        m_subscribes.insert(subscribeHandler.Key, subscribeHandler.Handler);
        return *this;
    }

    Dispatcher& operator-=(Observer observer)
    {
        QMutexLocker lock(&m_mutex);
        m_subscribes.remove(observer);
        return *this;
    }

private:
    QHash<Observer, FAction> m_subscribes;
    mutable QMutex m_mutex;
};

class DispatchersConnections : private Stack<Dispatcher*>
{
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
};

#endif // NOTIFICATION_H
