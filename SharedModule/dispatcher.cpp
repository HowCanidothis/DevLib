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
    if(m_disconnector != nullptr) {
        m_disconnector();
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
    m_disconnector();
}

DispatcherConnection& DispatcherConnection::operator+=(const DispatcherConnection& another)
{
    auto disconnector = m_disconnector;
    auto anotherDisconnector = another.m_disconnector;
    m_disconnector = [disconnector, anotherDisconnector]{
        disconnector();
        anotherDisconnector();
    };
    return *this;
}

DispatcherConnectionSafePtr DispatcherConnection::MakeSafe()
{
    auto result = ::make_shared<DispatcherConnectionSafe>(m_disconnector);
    m_registrator(result);
    return result;
}
