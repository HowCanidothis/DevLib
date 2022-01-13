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

    template<class T>
    void Write(const T& query)
    {
        NetworkPackage package;
        package.Pack(query);
        Write(package);
    }

    void Write(const NetworkPackage& package);

    void Connect(const QHostAddress& host, quint16 port);
    AsyncResult ConnectWithResult(const QHostAddress& host, quint16 port);
    void Disconnect() { m_socket.disconnectFromHost(); }
    bool IsConnected() const { return m_socket.state() == QTcpSocket::ConnectedState; }

    void SetSocketDescriptor(quintptr descriptor);
    qintptr GetSocketDescriptor() const { return m_socketDescriptor; }

    static bool LessThan(NetworkConnection* f, NetworkConnection* s);

signals:
    void disconnected();

private slots:
    void onReadyRead();
    void onSocketStateChanged(QAbstractSocket::SocketState);

private:
    friend class NetworkPackage;
    AsyncResult m_connectionResult;
    qintptr m_socketDescriptor;
    QTcpSocket m_socket;
    NetworkPackage m_currentPackage;
    qint32 m_redOffset;
    INetworkConnectionOutput* m_output;
};


#endif // NETWORKCONNECTION_H
