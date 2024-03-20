#include "dispatcher.h"

#include <QThread>

DispatcherConnectionSafe::DispatcherConnectionSafe()
{

}

DispatcherConnectionSafe::DispatcherConnectionSafe(const DispatcherConnection& connection)
    : m_connection(connection)
{
}

DispatcherConnectionSafe::~DispatcherConnectionSafe()
{
    Disconnect();
}

void DispatcherConnectionSafe::Disconnect()
{
    m_connection.disconnect();
}

DispatcherConnection::DispatcherConnection(const DispatcherGuardPtr& guard, const FAction& disconnector)
    : m_disconnector(disconnector)
    , m_guard(guard)
{}

DispatcherConnection::DispatcherConnection()
    : m_disconnector([]{})
{}

void DispatcherConnection::disconnect()
{
    if(m_disconnector == nullptr) {
        return;
    }
    if(m_guard == nullptr) {
        if(m_disconnector != nullptr) {
            m_disconnector();
            m_disconnector = nullptr;
        }
        return;
    }

    if(m_guard->Mutex != nullptr) {
        QMutexLocker locker(m_guard->Mutex.get());
        if(!m_guard->Destroyed) {
            m_disconnector();
            m_disconnector = nullptr;
        }
    } else if(!m_guard->Destroyed) {
        m_disconnector();
        m_disconnector = nullptr;
    }
}

DispatcherConnection::DispatcherConnection(const FAction& disconnector)
    : m_disconnector(disconnector)
{

}

DispatcherConnectionSafePtr DispatcherConnection::MakeSafe()
{
    return ::make_shared<DispatcherConnectionSafe>(*this);
}

void DispatcherConnections::MakeSafe(DispatcherConnectionsSafe& safeConnections)
{
    for(auto& connection : *this) {
        connection.MakeSafe(safeConnections);
    }
}

DispatcherConnectionsSafe DispatcherConnections::MakeSafe()
{
    DispatcherConnectionsSafe result;
    for(auto& connection : *this) {
        connection.MakeSafe(result);
    }
    return result;
}
