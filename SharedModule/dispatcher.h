#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QHash>
#include <QMutex>

#include <functional>

#include "SharedModule/MemoryManager/memorymanager.h"
#include "smartpointersadapters.h"
#include "stack.h"

#ifdef QT_DEBUG
#define CONNECTION_DEBUG_LOCATION __FILE__ QT_STRINGIFY(__LINE__)
#else
#define CONNECTION_DEBUG_LOCATION nullptr
#endif

class DispatcherConnection;
using DispatcherConnectionSafePtr = SharedPointer<class DispatcherConnectionSafe>;

class DispatcherConnectionSafe
{
    using Super = FAction;
public:
    DispatcherConnectionSafe();
    DispatcherConnectionSafe(const FAction& disconnector);
    ~DispatcherConnectionSafe();

private:
    template<typename... Args> friend class CommonDispatcher;
    void disable();

private:
    FAction m_disconnector;
};

using DispatcherConnectionsSafe = QVector<DispatcherConnectionSafePtr>;

inline SharedPointer<DispatcherConnectionsSafe> DispatcherConnectionsSafeCreate()
{
    return ::make_shared<DispatcherConnectionsSafe>();
}

class DispatcherConnection
{
    using FDispatcherRegistrator = std::function<void (const DispatcherConnectionSafePtr&)>;

    friend class DispatcherConnectionSafe;
    template<typename ... Args> friend class CommonDispatcher;
    FAction m_disconnector;
    FDispatcherRegistrator m_registrator;

    DispatcherConnection(const FAction& disconnector, const FDispatcherRegistrator& registrator);

public:
    DispatcherConnection();

    void Disconnect() const;
    DispatcherConnection& operator+=(const DispatcherConnection& another);

    DispatcherConnectionSafePtr MakeSafe();

    template<typename ... SafeConnections>
    void MakeSafe(SafeConnections&... connections)
    {
        auto connection = MakeSafe();
        adapters::Combine([&connection](DispatcherConnectionsSafe& connections){
            connections.append(connection);
        }, connections...);
    }
};

template<class T, class T2> class LocalProperty;
template<class T> struct LocalPropertyOptional;

template<typename ... Args>
class CommonDispatcher
{
public:
    using Type = void (Args...);
    using Observer = const void*;
    using FCommonDispatcherAction = std::function<Type>;
    using FCommonDispatcherActionWithResult = std::function<bool (Args...)>;

    struct ConnectionSubscribe
    {
        QHash<qint32, FCommonDispatcherAction> Subscribes;
        qint32 LastId = 0;
    };

    struct ActionHandler
    {
        Observer Key;
        FCommonDispatcherAction Handler;
    };

    virtual ~CommonDispatcher()
    {
        for(const auto& connection : m_safeConnections) {
            if(!connection.expired()) {
                connection.lock()->disable();
            }
        }
    }

    bool IsEmpty() const
    {
        return m_subscribes.isEmpty();
    }

    virtual void Invoke(Args... args) const
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

    DispatcherConnection ConnectFromWithParameters(const char* connectionInfo, CommonDispatcher& another) const
    {
        return another.Connect(this, [this, connectionInfo](Args... args){
            Invoke(args...);
        });
    }

    DispatcherConnection ConnectBoth(const char* connectionDescription, CommonDispatcher& another) const
    {
        auto sync = ::make_shared<std::atomic_bool>(false);
        auto result = another.Connect(this, [this, sync, connectionDescription](Args... args){
            if(!*sync) {
                *sync = true;
                Invoke(args...);
                *sync = false;
            }
        });
        result += Connect(this, [this, &another, sync, connectionDescription](Args... args){
            if(!*sync) {
                *sync = true;
                another.Invoke(args...);
                *sync = false;
            }
        });
        return result;
    }

    DispatcherConnection ConnectAction(const char* connectionInfo, const FAction& handler) const
    {
        return Connect(this, [handler, connectionInfo](Args...) { handler(); });
    }

    template<typename ... Dispatchers>
    DispatcherConnection ConnectCombined(const FAction& handler, Dispatchers&... args) const
    {
        DispatcherConnection result;
        adapters::Combine([this, &result, handler](const auto& target){
            result += target.ConnectAction(CONNECTION_DEBUG_LOCATION, handler);
        }, *this, args...);
        return result;
    }

    template<typename Disp, typename ... Dispatchers>
    DispatcherConnection ConnectFrom(const char* locationInfo, const Disp& another, Dispatchers&... args) const
    {
        DispatcherConnection result;
        adapters::Combine([this, &result, locationInfo](const auto& target){
            result += target.ConnectAction(locationInfo, [this, locationInfo]{ Invoke(); });
        }, another, args...);
        return result;
    }

    template<typename ... Dispatchers>
    DispatcherConnection ConnectAndCallCombined(const FAction& handler, Dispatchers&... args) const
    {
        auto ret = ConnectCombined(handler, args...);
        handler();
        return ret;
    }

    DispatcherConnection Connect(Observer key, const FCommonDispatcherAction& handler) const
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
            FCommonDispatcherAction subscribe;
            QMutexLocker locker(&m_mutex);
            auto foundIt = m_connectionSubscribes.find(key);
            if(foundIt != m_connectionSubscribes.end()) {
                auto& subscribes = foundIt.value().Subscribes;
                auto subscribeIt = subscribes.find(id);
                if(subscribeIt != subscribes.end()) {
                    subscribe = *subscribeIt;
                    subscribes.remove(id);
                }
            }
            m_safeConnections.remove(id);
        }, [this, id](const DispatcherConnectionSafePtr& connection){
            QMutexLocker lock(&m_mutex);
            m_safeConnections.insert(id, std::weak_ptr<DispatcherConnectionSafe>(connection));
        });
    }

    DispatcherConnection ConnectAndCall(Observer, const FAction& handler) const
    {
        auto result = ConnectAction(CONNECTION_DEBUG_LOCATION, handler);
        handler();
        return result;
    }

    void Disconnect(const DispatcherConnection& connection) const
    {
        connection.Disconnect();
    }

    const CommonDispatcher& operator+=(const ActionHandler& subscribeHandler) const
    {
        QMutexLocker lock(&m_mutex);
        Q_ASSERT(!m_subscribes.contains(subscribeHandler.Key));
        m_subscribes.insert(subscribeHandler.Key, subscribeHandler.Handler);
        return *this;
    }

    void operator-=(Observer observer) const
    {
        FCommonDispatcherAction action;
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
        }
    }

    SharedPointer<DispatcherConnectionsSafe> OnFirstInvoke(const FCommonDispatcherAction& action) const
    {
        auto connections = ::make_shared<DispatcherConnectionsSafe>();
        Connect(this, [action, connections](Args... args){
            action(args...);
            connections->clear();
        }).MakeSafe(*connections);
        return connections;
    }

private:
    friend class Connection;
    mutable QHash<qint32, std::weak_ptr<DispatcherConnectionSafe>> m_safeConnections;
    mutable QHash<Observer, ConnectionSubscribe> m_connectionSubscribes;
    mutable QHash<Observer, FCommonDispatcherAction> m_subscribes;
    mutable QMutex m_mutex;
};

using Dispatcher = CommonDispatcher<>;

Q_DECLARE_METATYPE(SharedPointer<DispatcherConnectionsSafe>)

#endif // NOTIFICATION_H
