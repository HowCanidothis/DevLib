#include "networkthread.h"

#include <QTimer>

#include "NetworkModule/networkconnection.h"
#include "NetworkModule/networkserverbase.h"

NetworkThread::NetworkThread(NetworkServerBase* server)
    : QThread(server)
    , m_server(server)
    , m_removeInvalidConnectionsTimer(new QTimer())
    , m_whileDeleting(false)
    , m_connectionsCounter(0)
{
    m_threadWorker.moveToThread(this);
    m_removeInvalidConnectionsTimer->start(2000);

    connect(m_removeInvalidConnectionsTimer.data(), SIGNAL(timeout()), this, SLOT(onRemoveInvalidConnections()));
}

NetworkThread::~NetworkThread()
{
    m_whileDeleting = true;
    quit();
    wait();
}

void NetworkThread::Write(qintptr descriptor, const NetworkPackage& package)
{
    ThreadsBase::DoQThreadWorker(CONNECTION_DEBUG_LOCATION, &m_threadWorker, [descriptor, package, this]{
        auto find = m_connections.FindSortedByPredicate(descriptor, [](NetworkConnection* connection, qintptr d){
                return connection->GetSocketDescriptor() < d;
        });

        if(find != m_connections.end()) {
            NetworkConnection* connection = *find;
            connection->Write(package);
        }
    });
}

void NetworkThread::AddSocket(qintptr descriptor)
{
    ++m_connectionsCounter;

    ThreadsBase::DoQThreadWorker(CONNECTION_DEBUG_LOCATION, &m_threadWorker, [descriptor, this]{
        NetworkConnection* connection = new NetworkConnection(m_server);
        connection->SetSocketDescriptor(descriptor);

        m_connections.InsertSortedUnique(connection, &NetworkConnection::LessThan);

        connect(connection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    });
}

void NetworkThread::onDisconnected()
{
    // this function is processing only in this thread, since NetworkConnections are created here

    if(m_whileDeleting) {
        return;
    }
    NetworkConnection* connection = reinterpret_cast<NetworkConnection*>(sender());
    qintptr socketDescriptor = connection->GetSocketDescriptor();
    ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION, [this, socketDescriptor]{
        m_server->onConnectionLost(socketDescriptor);
    });

    m_connectionsToRemove.insert(connection);

    --m_connectionsCounter;
}

void NetworkThread::onRemoveInvalidConnections()
{    
    ThreadsBase::DoQThreadWorker(CONNECTION_DEBUG_LOCATION, &m_threadWorker, [this]{
        m_connections.RemoveByPredicate([this](NetworkConnection* connection){
            if(m_connectionsToRemove.contains(connection)) {
                delete connection;
                return true;
            }
            return false;
        });
        m_connectionsToRemove.clear();
    });
}
