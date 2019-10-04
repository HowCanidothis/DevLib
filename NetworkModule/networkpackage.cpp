#include "networkpackage.h"

#include <QTcpSocket>

#include "networkconnection.h"

bool NetworkPackage::CheckSum() const
{
    return true;
    // return m_header.Hashsum == GenerateCheckSum();
}

QByteArray NetworkPackage::ToByteArray() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream.writeBytes((const char*)this, sizeof(NetworkPackageHeader));
    stream.writeBytes(m_data.data(), m_data.size());

    return array;
}

void NetworkPackage::Pack(const QByteArray& data)
{
    m_data = data;
    pack();
}

qint32 NetworkPackage::GenerateCheckSum() const
{
    quint32 hashSum = 0;
    for(const char byte: m_data) {
        hashSum += byte;
    }
    hashSum *= 0x1021;
    return qint32(hashSum);
}

void NetworkPackage::write(NetworkConnection* connection) const
{
    connection->m_socket.write((const char*)this, sizeof(NetworkPackageHeader));
    connection->m_socket.write(m_data.data(), m_data.size());
}

void NetworkPackage::pack()
{
    m_header.Size = m_data.size();
    m_header.Hashsum = GenerateCheckSum();
}

