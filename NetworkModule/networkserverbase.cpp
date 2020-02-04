#include "networkserverbase.h"

#include "Private/networkthreadpool.h"
#include "networkconnection.h"

NetworkServerBase::NetworkServerBase(qint32 threadsCount, QObject* parent)
    : QTcpServer(parent)
    , m_threads(new NetworkThreadPool(this, threadsCount))
    , m_host(QHostAddress::LocalHost)
    , m_port(1234)
{

}

NetworkServerBase::~NetworkServerBase()
{
}

void NetworkServerBase::StartServer()
{
    if(m_threads->IsRunning()) {
        m_threads->Quit();
        this->close();
    }

    if(this->listen(QHostAddress(m_host), m_port))
    {
        m_threads->Start();
        qInfo() << QString("Server started host: %1, port: %2").arg(m_host.toString(), QString::number(m_port));
    }
    else
    {
        m_threads->Quit();
        qWarning() << QString("Server did not start! Host: %1, port: %2").arg(m_host.toString(), QString::number(m_port));
    }
}

bool NetworkServerBase::IsRunning() const
{
    return m_threads->IsRunning();
}

void NetworkServerBase::incomingConnection(qintptr handle)
{
    m_threads->AddSocket(handle);
}

void NetworkServerBase::write(qintptr descriptor, const NetworkPackage& package)
{
    m_threads->Write(descriptor, package);
}
