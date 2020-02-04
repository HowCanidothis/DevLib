#include "networkconnection.h"

#include <QTimer>

#include "NetworkModule/networkserverbase.h"

NetworkConnection::NetworkConnection(INetworkConnectionOutput* output)
    : m_redOffset(0)
    , m_output(output)
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
    timer->singleShot(2000, [this, timer, host, port]{
        if(m_socket.state() != QTcpSocket::ConnectedState) {
            qCritical() << "Cannot connect to host, no response" << host << port;
        } else {
            qInfo() << "Successfuly connected to" << host << port;
        }
        timer->deleteLater();
    });
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
    auto& header = m_currentPackage.m_header;
    auto& data = m_currentPackage.m_data;
    auto socketDescriptor = m_socket.socketDescriptor();

    if(header.Size == 0) {
        if(m_socket.bytesAvailable() >= sizeof(NetworkPackageHeader)) {
            m_socket.read((char*)&header.SyncBytes, sizeof(qint16));

            qInfo() << socketDescriptor << "reading sync bytes...";

            if(!header.IsSynchronized()) {
                qWarning() << "incorrect sync bytes, finding a begin...";
                OnReadyRead();
                return;
            }

            m_socket.read((char*)&header.Size, sizeof(qint32));
            m_socket.read((char*)&header.Hashsum, sizeof(qint32));
            m_currentPackage.m_data.resize(header.Size);

            qInfo() << socketDescriptor << "parsing header...\n" << header;

            OnReadyRead();
        }
    } else if(m_socket.bytesAvailable()){
        while (header.Size != 0) {
            qint64 bytesRed = m_socket.read(data.data() + m_redOffset, header.Size);
            m_redOffset += bytesRed;
            header.Size -= bytesRed;

            qInfo() << socketDescriptor << "parsing package body...\n" << header;

            if(header.Size == 0) {
                if(m_currentPackage.CheckSum()) {
                    NetworkPackage packageCopy = m_currentPackage;
                    quintptr descriptor = m_socket.socketDescriptor();
                    INetworkConnectionOutput* output = m_output;

                    qInfo() << socketDescriptor << "package received...";
                    output->onPackageRecieved(descriptor, packageCopy);
                    header.SyncBytes = 0x0;
                }
                else
                {
                    qWarning() << socketDescriptor << "incorrect hash sum\n" << m_currentPackage.m_header;
                    qWarning() << "Calculated checkSum:" << m_currentPackage.GenerateCheckSum();
                }

                m_redOffset = 0;
            }
        }

        OnReadyRead();
    }
}
