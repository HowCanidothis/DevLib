#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QHash>
#include <QMutex>

#include <functional>

#include "smartpointersadapters.h"
#include "stack.h"

template<typename ... Args>
class CommonDispatcher ATTACH_MEMORY_SPY(CommonDispatcher<Args...>)
{
public:
    enum SubscribeMode {
        RepeatableSameSubscribe
    };
    using Type = void (Args...);
    using FCommonDispatcherAction = std::function<Type>;
    using Observer = void*;

    struct ConnectionSubscribe
    {
        QHash<qint32, FCommonDispatcherAction> Subscribes;
        qint32 LastId = 0;
    };

    class Connection
    {
        friend class CommonDispatcher;
        CommonDispatcher* m_dispatcher;
        Observer m_key;
        qint32 m_id;

        Connection(CommonDispatcher* dispatcher, Observer observer, qint32 id)
            : m_dispatcher(dispatcher)
            , m_key(observer)
            , m_id(id)
        {}
    private:
        void Disconnect()
        {
            m_dispatcher->Disconnect(*this);
        }
    };

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
        QHash<Observer, FCommonDispatcherAction> subscribesCopy;
        {
            QMutexLocker locker(&m_mutex);
            subscribesCopy = m_subscribes;
        }
        for(const auto& subscribe : subscribesCopy)
        {
            subscribe(args...);
        }
        QHash<Observer, RepeatableSameSubscribeAction> multiSubscribesCopy;
        {
            QMutexLocker locker(&m_mutex);
            multiSubscribesCopy = m_multiSubscribes;
        }
        for(const auto& subscribe : multiSubscribesCopy)
        {
            subscribe.Handler(args...);
        }
        QHash<Observer, ConnectionSubscribe> connectionSubscribesCopy;
        {
            QMutexLocker locker(&m_mutex);
            connectionSubscribesCopy = m_connectionSubscribes;
        }
        for(const auto& connections : connectionSubscribesCopy)
        {
            for(const auto& subscribe : connections.Subscribes) {
                subscribe(args...);
            }
        }
    }

    void operator()(Args... args) const
    {
        Invoke(args...);
    }

    Connection Connect(Observer key, const FCommonDispatcherAction& handler)
    {
        QMutexLocker lock(&m_mutex);
        auto foundIt = m_connectionSubscribes.find(key);
        if(foundIt == m_connectionSubscribes.end()) {
            foundIt = m_connectionSubscribes.insert(key, ConnectionSubscribe());
        }
        ConnectionSubscribe& connectionSubscribe = foundIt.value();
        connectionSubscribe.Subscribes.insert(connectionSubscribe.LastId++, handler);
        return Connection(this, key, connectionSubscribe.LastId - 1);
    }

    void Disconnect(const Connection& connection)
    {
        QMutexLocker lock(&m_mutex);
        auto foundIt = m_connectionSubscribes.find(connection.m_key);
        if(foundIt != m_connectionSubscribes.end()) {
            foundIt.value().Subscribes.remove(connection.m_id);
        }
    }

    CommonDispatcher& operator+=(const ActionHandler& subscribeHandler)
    {
        QMutexLocker lock(&m_mutex);
        Q_ASSERT(!m_subscribes.contains(subscribeHandler.Key));
        m_subscribes.insert(subscribeHandler.Key, subscribeHandler.Handler);
        return *this;
    }

    void operator-=(Observer observer)
    {
        FCommonDispatcherAction action;
        RepeatableSameSubscribeAction multiAction;
        ConnectionSubscribe connection;
        {
            QMutexLocker lock(&m_mutex);
            {
                auto foundIt = m_subscribes.find(observer);
                if(foundIt != m_subscribes.end()) {
                    action = foundIt.value();
                    m_subscribes.erase(foundIt);
                }
            }
            {
                auto foundIt = m_connectionSubscribes.find(observer);
                if(foundIt != m_connectionSubscribes.end()) {
                    connection = foundIt.value();
                    m_connectionSubscribes.erase(foundIt);
                }
            }
            m_connectionSubscribes.remove(observer);
            auto foundIt = m_multiSubscribes.find(observer);
            if(foundIt != m_multiSubscribes.end()) {
                if(--foundIt.value().Counter == 0) {
                    multiAction = foundIt.value();
                    m_multiSubscribes.erase(foundIt);
                }
            }
        }
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
    friend class Connection;
    QHash<Observer, ConnectionSubscribe> m_connectionSubscribes;
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
