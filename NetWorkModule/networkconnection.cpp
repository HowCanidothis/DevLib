#include "networkconnection.h"

#include <QTimer>

#include "NetworkModule/networkserverbase.h"

NetworkConnection::NetworkConnection(INetworkConnectionOutput* output)
    : m_output(output)
{
    m_currentPackage.m_data.reserve(0xffff); // 65 kB

    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
    connect(&m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

void NetworkConnection::Write(const NetworkPackage& package)
{
    if(m_socket.state() != QTcpSocket::ConnectedState) {
        qCritical() << "Unable to send data. No connection";
        return;
    }
    package.write(this);
}

void NetworkConnection::Connect(const QHostAddress& host, quint16 port)
{
    m_socket.connectToHost(host, port);
    QTimer* timer = new QTimer;
    connect(timer, &QTimer::timeout, [this, timer, host, port]{
        if(m_socket.state() != QTcpSocket::ConnectedState) {
            qCritical() << "Cannot connect to host, no response" << host << port;
        } else {
            qInfo() << "Successfuly connected to" << host << port;
        }
        timer->deleteLater();
    });
    timer->start(1000);
}

void NetworkConnection::SetSocketDescriptor(quintptr descriptor)
{
    m_socketDescriptor = descriptor;
    m_socket.setSocketDescriptor(descriptor);
}

bool NetworkConnection::LessThan(NetworkConnection* f, NetworkConnection* s)
{
    return f->GetSocketDescriptor() < s->GetSocketDescriptor();
}

void NetworkConnection::OnReadyRead()
{
    auto& data = m_currentPackage.m_data;
    auto& size = m_currentPackage.m_size;
    auto& hashSum = m_currentPackage.m_hashsum;

    if(size == 0) {
        if(m_socket.bytesAvailable() >= (sizeof(qint32) * 2)) {
            m_socket.read((char*)&size, sizeof(qint32));
            m_socket.read((char*)&hashSum, sizeof(qint32));
            m_currentPackage.m_data.resize(size);
        }
    }
    if(m_socket.bytesAvailable()){
        qint64 bytesRed = m_socket.read(data.data(), size);
        size -= bytesRed;
        if(size == 0) {
            if(m_currentPackage.CheckSum()) {
                NetworkPackage packageCopy = m_currentPackage;
                quintptr descriptor = m_socket.socketDescriptor();
                INetworkConnectionOutput* output = m_output;
                ThreadsBase::DoMain([output, packageCopy, descriptor]{
                    output->onPackageRecieved(descriptor, packageCopy);
                });
            }
        }
    }
}
