#ifndef NETWORKSERVERBASE_H
#define NETWORKSERVERBASE_H

#include <QTcpServer>

#include "network_decl.h"

class NetworkPackage;

class NetworkServerBase : public QTcpServer, INetworkConnectionOutput
{
    Q_OBJECT
public:
    explicit NetworkServerBase(QObject *parent = 0);
    virtual ~NetworkServerBase() Q_DECL_OVERRIDE;
    void StartServer();
    bool IsRunning() const;

    void SetHostPort(quint16 port) { m_port = port; }
    void SetHost(const QString& host) { m_host = QHostAddress(host); }

protected:
    void incomingConnection( qintptr handle ) final;
    void write(qintptr descriptor, const NetworkPackage& package);

private:
    friend class NetworkThread;
    class NetworkThread *m_thread;
    QHostAddress m_host;
    quint16 m_port;
};

#endif // NETWORKSERVERBASE_H
