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
    enum SubscribeMode {
        RepeatableSameSubscribe
    };
    using FCommonDispatcherAction = std::function<void (Args...)>;
    using Observer = void*;
    struct RepeatableSameSubscribeAction
    {
        FCommonDispatcherAction Handler;
        qint32 Counter = 1;
    };
    struct RepeatableSameSubscribeUnsubscriber
    {
        SubscribeMode Multi;
        Observer Key;
    };

    struct ActionHandler
    {
        Observer Key;
        FCommonDispatcherAction Handler;
    };
    struct RepeatableSameSubscribeHandler
    {
        SubscribeMode Multi;
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
        for(const auto& subscribe : m_subscribes)
        {
            subscribe(args...);
        }
        for(const auto& subscribe : m_multiSubscribes)
        {
            subscribe.Handler(args...);
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
        auto foundIt = m_multiSubscribes.find(observer);
        if(foundIt != m_multiSubscribes.end()) {
            if(--foundIt.value().Counter == 0) {
                m_multiSubscribes.erase(foundIt);
            }
        }
        return *this;
    }

    CommonDispatcher& operator-=(const RepeatableSameSubscribeUnsubscriber& unsubscriber)
    {
        QMutexLocker lock(&m_mutex);
        auto foundIt = m_multiSubscribes.find(unsubscriber.Key);
        if(foundIt != m_multiSubscribes.end()) {
            if(--foundIt.value().Counter == 0) {
                m_multiSubscribes.erase(foundIt);
            }
        }
        return *this;
    }


    CommonDispatcher& operator+=(const RepeatableSameSubscribeHandler& subscribeHandler)
    {
        QMutexLocker lock(&m_mutex);
        auto foundIt = m_multiSubscribes.find(subscribeHandler.Key);
        if(foundIt == m_multiSubscribes.end()) {
            foundIt = m_multiSubscribes.insert(subscribeHandler.Key, { subscribeHandler.Handler });
        }
        foundIt.value().Counter++;
        return *this;
    }

private:
    QHash<Observer, FCommonDispatcherAction> m_subscribes;
    QHash<Observer, RepeatableSameSubscribeAction> m_multiSubscribes;
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
