#include "dispatcher.h"

DispatcherConnectionSafe::DispatcherConnectionSafe()
{

}

DispatcherConnectionSafe::DispatcherConnectionSafe(const FAction& disconnector)
    : m_disconnector(disconnector)
{
}

DispatcherConnectionSafe::~DispatcherConnectionSafe()
{
    Disconnect();
}

void DispatcherConnectionSafe::Disconnect()
{
    if(m_disconnector != nullptr) {
        m_disconnector();
        m_disconnector = nullptr;
    }
}

void DispatcherConnectionSafe::disable()
{
    m_disconnector = nullptr;
}

DispatcherConnection::DispatcherConnection(const FAction& disconnector, const FDispatcherRegistrator& registrator)
    : m_disconnector(disconnector)
    , m_registrator(registrator)
{}

DispatcherConnection::DispatcherConnection()
    : m_disconnector([]{})
    , m_registrator([](const DispatcherConnectionSafePtr&){})
{}

void DispatcherConnection::Disconnect() const
{
    if(m_disconnector != nullptr) {
        m_disconnector();
    }
}

void DispatcherConnections::Disconnect()
{
    for(const auto& connection : *this)
    {
        connection.Disconnect();
    }
    clear();
}

DispatcherConnectionSafePtr DispatcherConnection::MakeSafe()
{
    Q_ASSERT(m_registrator != nullptr);
    auto result = ::make_shared<DispatcherConnectionSafe>(m_disconnector);
    m_registrator(result);
    m_registrator = nullptr;
    return result;
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
