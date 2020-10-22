#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QHash>
#include <QMutex>

#include <functional>

#include "SharedModule/MemoryManager/memorymanager.h"
#include "smartpointersadapters.h"
#include "stack.h"

class DispatcherConnection;
class DispatcherConnectionSafePtr;

using FDispatcherRegistrator = std::function<void (const DispatcherConnectionSafePtr&)>;

class DispatcherConnectionSafePtr : public SharedPointer<DispatcherConnection>
{
    using Super = SharedPointer<DispatcherConnection>;
public:
    DispatcherConnectionSafePtr() {}
    DispatcherConnectionSafePtr(const FAction& disconnector, const FDispatcherRegistrator& registrator);
    ~DispatcherConnectionSafePtr();
};

inline uint qHash(const DispatcherConnectionSafePtr& connection, uint seed = 0)
{
    return qHash(connection.get(), seed);
}

using DispatcherConnectionsSafe = QVector<DispatcherConnectionSafePtr>;

class DispatcherConnection
{
    friend class DispatcherConnectionSafePtr;
    template<typename ... Args> friend class CommonDispatcher;
    FAction m_disconnector;
    FDispatcherRegistrator m_registrator;

    DispatcherConnection(const FAction& disconnector, const FDispatcherRegistrator& registrator)
        : m_disconnector(disconnector)
        , m_registrator(registrator)
    {}

public:
    DispatcherConnection& Add(const DispatcherConnection& another)
    {
        auto anotherDisconnector = another.m_disconnector;
        auto oldDisconnector = m_disconnector;
        m_disconnector = [oldDisconnector, anotherDisconnector]{
            oldDisconnector();
            anotherDisconnector();
        };
        return *this;
    }

    void Disconnect() const
    {
        m_disconnector();
    }

    DispatcherConnectionSafePtr MakeSafe() { return DispatcherConnectionSafePtr(m_disconnector, m_registrator); }
    void MakeSafe(DispatcherConnectionsSafe& connections)
    {
        connections.append(MakeSafe());
    }
};

inline DispatcherConnectionSafePtr::DispatcherConnectionSafePtr(const FAction& disconnector, const FDispatcherRegistrator& registrator)
    : Super(new DispatcherConnection(disconnector, registrator))
{
    registrator(*this);
}

inline DispatcherConnectionSafePtr::~DispatcherConnectionSafePtr()
{
    if(get() != nullptr && use_count() == 2) {
        get()->Disconnect();
    }
}


template<typename ... Args>
class CommonDispatcher ATTACH_MEMORY_SPY(CommonDispatcher<Args...>)
{
public:
    enum SubscribeMode {
        RepeatableSameSubscribe
    };
    using Type = void (Args...);
    using Observer = void*;
    using FCommonDispatcherAction = std::function<Type>;


    struct ConnectionSubscribe
    {
        QHash<qint32, FCommonDispatcherAction> Subscribes;
        qint32 LastId = 0;
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

    ~CommonDispatcher()
    {
        m_safeConnections.clear();
    }

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

    DispatcherConnection Connect(CommonDispatcher& another)
    {
        return another.Connect(this, [this](Args... args){
            Invoke(args...);
        });
    }

    DispatcherConnection Connect(Observer key, const FCommonDispatcherAction& handler)
    {
        QMutexLocker lock(&m_mutex);
        auto foundIt = m_connectionSubscribes.find(key);
        if(foundIt == m_connectionSubscribes.end()) {
            foundIt = m_connectionSubscribes.insert(key, ConnectionSubscribe());
        }
        ConnectionSubscribe& connectionSubscribe = foundIt.value();
        connectionSubscribe.Subscribes.insert(connectionSubscribe.LastId++, handler);
        qint32 id = connectionSubscribe.LastId - 1;
        return DispatcherConnection([this, key, id]{
            QMutexLocker lock(&m_mutex);
            auto foundIt = m_connectionSubscribes.find(key);
            if(foundIt != m_connectionSubscribes.end()) {
                foundIt.value().Subscribes.remove(id);
            }
        }, [this](const DispatcherConnectionSafePtr& connection){
            QMutexLocker lock(&m_mutex);
            m_safeConnections.insert(connection);
        });
    }

    void Disconnect(const DispatcherConnection& connection)
    {
        connection.Disconnect();
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
    QSet<DispatcherConnectionSafePtr> m_safeConnections;
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
