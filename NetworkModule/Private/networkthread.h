#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H

#include <SharedModule/internal.hpp>

class NetworkThread : public QThread
{
    Q_OBJECT
    class NetworkServerBase* m_server;
public:
    NetworkThread(class NetworkServerBase* server);
    ~NetworkThread();

    void Write(qintptr descriptor, const class NetworkPackage& package);

    void AddSocket(qintptr descriptor);
    qint32 GetConnectionsCount() const { return m_connectionsCounter; }

private slots:
    void onDisconnected();
    void onRemoveInvalidConnections();

public:
    ArrayPointers<class NetworkConnection> m_connections;
    QSet<class NetworkConnection*> m_connectionsToRemove;
    ScopedPointer<class QTimer> m_removeInvalidConnectionsTimer;
    std::atomic_bool m_whileDeleting;
    std::atomic_int m_connectionsCounter;
    QObject m_threadWorker;
};
#endif // NETWORKTHREAD_H
