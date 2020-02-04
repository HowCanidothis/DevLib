#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QTcpSocket>

#include "NetworkModule/networkpackage.h"
#include "NetworkModule/network_decl.h"

class NetworkConnection : public QObject
{
    Q_OBJECT

public:
    NetworkConnection(INetworkConnectionOutput* output);

    void Write(const NetworkPackage& package);

    void Connect(const QHostAddress& host, quint16 port);
    void Disconnect() { m_socket.disconnectFromHost(); }

    void SetSocketDescriptor(quintptr descriptor);
    qintptr GetSocketDescriptor() const { return m_socketDescriptor; }

    static bool LessThan(NetworkConnection* f, NetworkConnection* s);

signals:
    void disconnected();

private slots:
    void OnReadyRead();

private:
    void processInput();
    void processInputThroughThreadPool();

private:
    friend class NetworkPackage;

    typedef void (NetworkConnection::*ProcessInputFunction)();

    qintptr m_socketDescriptor;
    QTcpSocket m_socket;
    NetworkPackage m_currentPackage;
    qint32 m_redOffset;
    INetworkConnectionOutput* m_output;
};


#endif // NETWORKCONNECTION_H
