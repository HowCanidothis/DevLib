#include "networkserverbase.h"

#include "Private/networkthread.h"
#include "networkconnection.h"

NetworkServerBase::NetworkServerBase(QObject *parent)
    : QTcpServer(parent)
    , m_thread(new NetworkThread(this))
    , m_host(QHostAddress::LocalHost)
    , m_port(1234)
{

}

NetworkServerBase::~NetworkServerBase()
{

}

void NetworkServerBase::StartServer()
{
    if(m_thread->isRunning()) {
        m_thread->quit();
        this->close();
    }

    if(this->listen(QHostAddress(m_host), m_port))
    {
        m_thread->start();
        qInfo() << "Server started";
    }
    else
    {
        m_thread->quit();
        qWarning() << "Server did not start!";
    }
}

bool NetworkServerBase::IsRunning() const
{
    return m_thread->isRunning();
}

void NetworkServerBase::incomingConnection(qintptr handle)
{
    m_thread->AddSocket(handle);
}

void NetworkServerBase::write(qintptr descriptor, const NetworkPackage& package)
{
    m_thread->Write(descriptor, package);
}
