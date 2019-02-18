#ifndef NETWORKPACKAGE_H
#define NETWORKPACKAGE_H

#include <QByteArray>

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

    qint32 GenerateCheckSum() const;
    const QByteArray& GetData() const { return m_data; }
    bool CheckSum() const;
    bool IsEmpty() const { return m_data.isEmpty(); }

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << m_size;
        buffer << m_hashsum;
        buffer << m_data;
    }

private:
    void pack();
    void write(class NetworkConnection* connection) const;

private:
    friend class NetworkConnection;
    qint32 m_size = 0;
    qint32 m_hashsum;
    QByteArray m_data;
};

#endif // SERVERPACKAGE_H
