#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QHash>
#include <QMutex>
#include <QQueue>

#include <functional>

#include "SharedModule/MemoryManager/memorymanager.h"
#include "smartpointersadapters.h"
#include "stack.h"

#define CONNECTION_DEBUG_LOCATION DEBUG_LOCATION
#define CDL CONNECTION_DEBUG_LOCATION

class DispatcherConnection;
class DispatcherConnectionSafe;
using DispatcherConnectionSafePtr = SharedPointer<DispatcherConnectionSafe>;

using DispatcherConnectionsSafe = QVector<DispatcherConnectionSafePtr>;

inline SharedPointer<DispatcherConnectionsSafe> DispatcherConnectionsSafeCreate()
{
    return ::make_shared<DispatcherConnectionsSafe>();
}

struct DispatcherGuard
{
    SharedPointer<QMutex> Mutex;
    bool Destroyed = false;
};

using DispatcherGuardPtr = SharedPointer<DispatcherGuard>;

class DispatcherConnection
{
    using FDispatcherRegistrator = std::function<void (const DispatcherConnectionSafePtr&)>;

    friend class DispatcherConnectionSafe;
    template<typename ... Args> friend class CommonDispatcher;
    template<typename... Args> friend class LocalDispatcher; // TODO. Must be removed
    FAction m_disconnector;
    DispatcherGuardPtr m_guard;

    DispatcherConnection(const DispatcherGuardPtr& guard, const FAction& disconnector);
    void disconnect();
public:
    DispatcherConnection(const FAction& disconnector);
    DispatcherConnection();

    DispatcherConnectionSafePtr MakeSafe();

    template<typename ... SafeConnections>
    void MakeSafe(SafeConnections&... connections);
};

class DispatcherConnectionSafe
{
    using Super = FAction;
public:
    DispatcherConnectionSafe();
    DispatcherConnectionSafe(const DispatcherConnection& disconnector);
    ~DispatcherConnectionSafe();

    void Disconnect();

private:
    DispatcherConnection m_connection;
};

template<typename ... SafeConnections>
inline void DispatcherConnection::MakeSafe(SafeConnections&... connections)
{
    auto connection = MakeSafe();
    adapters::Combine([&connection](DispatcherConnectionsSafe& connections){
        connections.append(::make_shared<DispatcherConnectionSafe>(DispatcherConnection([connection]{
            connection->Disconnect();
        })));
    }, connections...);
}

class DispatcherConnections : public QVector<DispatcherConnection>
{
    using Super = QVector<DispatcherConnection>;

public:
    using Super::Super;   

    void MakeSafe(DispatcherConnectionsSafe& safeConnections);

    template<typename ... SafeConnections>
    void MakeSafe(DispatcherConnectionsSafe& connections, SafeConnections&... additionalConnections)
    {
        for(auto& connection : *this) {
            connection.MakeSafe(connections, additionalConnections...);
        }
    }

    DispatcherConnectionsSafe MakeSafe();
};

template<class T> class LocalProperty;
template<class T> struct LocalPropertyOptional;


template<typename ... Args>
class CommonDispatcher
{
public:
    using Type = void (Args...);
    using Observer = const void*;
    using FCommonDispatcherAction = std::function<Type>;
    using FCommonDispatcherActionWithResult = std::function<bool (Args...)>;
    using ConnectionSubscribe = FCommonDispatcherAction;

    struct ActionHandler
    {
        Observer Key;
        FCommonDispatcherAction Handler;
    };

    CommonDispatcher()
        : m_lastId(0)
        , m_lock([this](const char* ci){
#ifdef QT_DEBUG
            if(m_thread == 0) {
                m_thread = (qint64)QThread::currentThreadId();
                return;
            }
            if((qint64)QThread::currentThreadId() != m_thread) {
                qDebug() << "Incorrect dispatcher usage at" << ci;
            }
#else
            Q_UNUSED(ci);
#endif
    })
        , m_unlock([]{})
        , m_guard(::make_shared<DispatcherGuard>())
    {
    }

    virtual ~CommonDispatcher()
    {
        reset();
    }

    void ResetThread(qint64 threadId = 0) const
    {
#ifdef QT_DEBUG
        m_thread = threadId;
#endif
    }

    void SetAutoThreadSafe(const SharedPointer<QMutex>& mutex)
    {
#ifdef QT_DEBUG
        m_thread = 0;
        Q_ASSERT(!m_multithread);
        m_multithread = true;
#endif
        m_guard->Mutex = mutex;
        auto* pMutex = mutex.get();
        auto locked = ::make_shared<bool>(false);
        m_lock = [pMutex, locked](const char*) {
            *locked = pMutex->tryLock();
        };
        m_unlock = [pMutex, locked]{
            if(*locked) {
                pMutex->unlock();
            }
        };
    }

    void SetAutoThreadSafe()
    {
#ifdef QT_DEBUG
        m_thread = 0;
        Q_ASSERT(!m_multithread);
        m_multithread = true;
#endif
        m_guard->Mutex = ::make_shared<QMutex>();
        auto* pMutex = m_guard->Mutex.get();
        m_lock = [pMutex](const char*) {
            pMutex->lock();
        };
        m_unlock = [pMutex]{
            pMutex->unlock();
        };
    }

    virtual void Invoke(Args... args) const
    {
        QHash<Observer, FCommonDispatcherAction> subscribesCopy;
        QVector<Connection> connectionSubscribesCopy;
        {
            lock(CONNECTION_DEBUG_LOCATION);
            subscribesCopy = m_subscribes;
            connectionSubscribesCopy = m_connections;
            unlock();
        }
        for(const auto& subscribe : ::make_const(subscribesCopy))
        {
            subscribe(args...);
        }
        for(const auto& connections : ::make_const(connectionSubscribesCopy))
        {
            connections.ConnectionHandler(args...);
        }
    }

    void operator()(Args... args) const
    {
        Invoke(args...);
    }

    DispatcherConnection ConnectFromWithParameters(const char* connectionInfo, CommonDispatcher& another) const
    {
        return another.Connect(connectionInfo, [this, connectionInfo](Args... args){
            Invoke(args...);
        });
    }

    DispatcherConnections ConnectBoth(const char* connectionDescription, CommonDispatcher& another) const
    {
        DispatcherConnections result;
        auto sync = ::make_shared<std::atomic_bool>(false);
        result += another.Connect(connectionDescription, [this, sync, connectionDescription](Args... args){
            if(!*sync) {
                *sync = true;
                Invoke(args...);
                *sync = false;
            }
        });
        result += Connect(connectionDescription, [this, &another, sync, connectionDescription](Args... args){
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
        return Connect(connectionInfo, [handler, connectionInfo](Args...) { handler(); });
    }

    template<typename ... Dispatchers>
    DispatcherConnections ConnectCombined(const char* connectionInfo, const FAction& handler, Dispatchers&... args) const
    {
        DispatcherConnections result;
        adapters::Combine([this, &result, handler, connectionInfo](const auto& target){
            result += target.ConnectAction(connectionInfo, handler);
        }, *this, args...);
        return result;
    }

    template<typename Disp, typename ... Dispatchers>
    DispatcherConnections ConnectFrom(const char* locationInfo, const Disp& another, Dispatchers&... args) const
    {
        DispatcherConnections result;
        adapters::Combine([this, &result, locationInfo](const auto& target){
            result += target.ConnectAction(locationInfo, [this, locationInfo]{ Invoke(); });
        }, another, args...);
        return result;
    }

    template<typename ... Dispatchers>
    DispatcherConnections ConnectAndCallCombined(const char* connectionInfo, const FAction& handler, Dispatchers&... args) const
    {
        auto ret = ConnectCombined(connectionInfo, handler, args...);
        handler();
        return ret;
    }

    DispatcherConnection Connect(const char* locationInfo, const FCommonDispatcherAction& handler) const
    {
        lock(locationInfo);
        qint32 id;
        if(!m_freeConnections.isEmpty()) {
            id = m_freeConnections.dequeue();
            m_connections[id].ConnectionHandler = handler;
        } else {
            id = m_connections.size();
            m_connections.append(Connection(handler));
        }
        unlock();
        return DispatcherConnection(m_guard, [this, id, locationInfo]{
            if(!m_connections.isDetached()) {
                m_connections = ::make_copy(m_connections);
            }
            auto& connection = m_connections[id];
            auto handler = connection.ConnectionHandler;
            Q_UNUSED(handler);
            connection.ConnectionHandler = [](Args...){};
            m_freeConnections.enqueue(id);
        });
    }

    DispatcherConnection ConnectAndCall(const char* connectionInfo, const FAction& handler) const
    {
        auto result = ConnectAction(connectionInfo, handler);
        handler();
        return result;
    }

    const CommonDispatcher& operator+=(const ActionHandler& subscribeHandler) const
    {
        lock(CONNECTION_DEBUG_LOCATION);
        Q_ASSERT(!m_subscribes.contains(subscribeHandler.Key));
        m_subscribes.insert(subscribeHandler.Key, subscribeHandler.Handler);
        unlock();
        return *this;
    }

    const CommonDispatcher& operator-=(Observer key) const
    {
        FCommonDispatcherAction action;
        lock(CONNECTION_DEBUG_LOCATION);
        {
            auto foundIt = m_subscribes.find(key);
            if(foundIt != m_subscribes.end()) {
                action = foundIt.value();
                m_subscribes.erase(foundIt);
            }
        }
        unlock();
        return *this;
    }

    DispatcherConnection OnFirstInvoke(const FCommonDispatcherAction& action) const
    {
        auto connections = DispatcherConnectionsSafeCreate();
        Connect(CONNECTION_DEBUG_LOCATION, [action, connections](Args... args){
            action(args...);
            connections->clear();
        }).MakeSafe(*connections);
        return DispatcherConnection([connections]{ connections->clear(); });
    }

//    template<typename ... SafeConnections>
//    void OnFirstInvoke(const char* connectionInfo, const FCommonDispatcherAction& action, DispatcherConnectionsSafe& firstConnections, SafeConnections&... connections) const
//    {
//        auto firstInvokeConnections = DispatcherConnectionsSafeCreate();
//        Connect(connectionInfo, [action, firstInvokeConnections](Args... args){
//            action(args...);
//            firstInvokeConnections->Clear();
//        }).MakeSafe(*firstInvokeConnections, firstConnections, connections...);
//    }

    Q_DISABLE_COPY(CommonDispatcher)

protected:
    friend class PromiseData;
    friend class FutureResultData;
    void lock(const char* connectionInfo) const { m_lock(connectionInfo); }
    void unlock() const { m_unlock(); }
    void reset()
    {
        lock(CONNECTION_DEBUG_LOCATION);
        m_guard->Destroyed = true;
        m_connections.clear();
        m_subscribes.clear();
        unlock();
    }

private:
    friend class Connection;
    struct Connection {
        FCommonDispatcherAction ConnectionHandler;

        Connection(const FCommonDispatcherAction& connection)
            : ConnectionHandler(connection)
        {}
        Connection(){}
    };
    mutable QVector<Connection> m_connections;
    mutable QQueue<qint32> m_freeConnections;
    mutable QHash<Observer, FCommonDispatcherAction> m_subscribes;
    mutable std::atomic_uint32_t m_lastId;
    std::function<void (const char*)> m_lock;
    FAction m_unlock;
    DispatcherGuardPtr m_guard;

#ifdef QT_DEBUG
    mutable qint64 m_thread = 0;
    bool m_multithread = false;
#endif
};

template<class ... Args>
class CommonDispatcherThreadSafe : public CommonDispatcher<Args...>
{
    using Super = CommonDispatcher<Args...>;
public:
    CommonDispatcherThreadSafe()
    {
        Super::SetAutoThreadSafe();
    }
};

using Dispatcher = CommonDispatcher<>;

DECLARE_WITH_FIELD(DispatcherConnectionsSafe, Connections);
DECLARE_WITH_FIELD(DispatcherConnections, ConnectionsUnsafe);

inline SharedPointer<Dispatcher> DispatcherCreate() { return ::make_shared<Dispatcher>(); }
Q_DECLARE_METATYPE(SharedPointer<DispatcherConnectionsSafe>)

#endif // NOTIFICATION_H
