#ifndef NETWORKPACKAGE_H
#define NETWORKPACKAGE_H

#include <QByteArray>

#include <SharedGuiModule/internal.hpp>

#pragma pack(1)

struct NetworkPackageHeader
{
    quint16 SyncBytes = 0xbad0;
    qint32 Size = 0;
    quint32 Hashsum;

    bool IsSynchronized() const { return SyncBytes == 0xbad0; }

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << SyncBytes;
        buffer << Size;
        buffer << Hashsum;
    }

    friend QDebug operator<<(QDebug out, const NetworkPackageHeader& header)
    {
        out << "NetworkPackageHeader\n";
        out << "(\n";
        out << "    Sync bytes:" << header.SyncBytes << "\n";
        out << "    Size:" << header.Size << "\n";
        out << "    HashSum:" << header.Hashsum << "\n";
        out << ")\n";
        return out;
    }
};

#pragma pack()

class NetworkPackage
{
public:

    void Pack(const QByteArray& data);

    template<class T>
    void Pack(const T& data)
    {
        QStreamBufferWrite writer(&m_data, QIODevice::WriteOnly);
        writer << data;
        pack();
    }

    template<class T>
    T Read() const
    {
        T data;
        QStreamBufferRead reader(m_data);
        reader << data;
        return data;
    }

    quint32 GenerateCheckSum() const;
    const QByteArray& GetData() const { return m_data; }
    bool CheckSum() const;
    bool IsEmpty() const { return m_data.isEmpty(); }
    QByteArray ToByteArray() const;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << m_header;
        buffer << m_data;
    }

private:
    void pack();
    void write(class NetworkConnection* connection) const;

private:
    friend class NetworkConnection;
    NetworkPackageHeader m_header;
    QByteArray m_data;
};

#endif // SERVERPACKAGE_H
