#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "stdserializer.h"
#include "SharedModule/flags.h"
#include "SharedModule/name.h"

class StandardVariantPropertiesContainer
{
    QHash<Name,QVariant> m_properties;
public:
    void SetProperty(const Name& propertyName, const QVariant& value)
    {
        m_properties.insert(propertyName, value);
    }
    const QVariant& GetProperty(const Name& propertyName) const
    {
        auto foundIt = m_properties.find(propertyName);
        if(foundIt != m_properties.end()) {
            return foundIt.value();
        }
        return Default<QVariant>::Value;
    }
};

template<class Stream>
class SerializerBufferBase
{
public:
    SerializerBufferBase(Stream* stream, bool isReading)
        : m_stream(stream)
        , m_version(-1)
        , m_mode(SerializationMode_Default)
        , m_isReading(isReading)
    {
    }

    void BeginArrayObject(){}
    void EndArrayObject(){}
    template<class Buffer>
    void BeginArray(Buffer& buffer, qint32& size)
    {
        buffer << size;
    }

    template<class Buffer>
    void BeginKeyValueArray(Buffer& buffer, qint32& size)
    {
        BeginArray(buffer, size);
    }

    template<class Buffer, typename T, typename T2>
    void KeyValue(Buffer& buffer, T& key, T2& value)
    {
        buffer << key;
        buffer << value;
    }

    void OpenSection(const QString&) {}
    template<class T>
    T& Attr(const QString&, T& value) const { return value; }
    template<class T>
    T& Sect(const QString&, T& value) const { return value; }
    template<class T>
    T& SectWithContext(const QString&, T& value, const struct TextConverterContext&) { return value; }

    void CloseSection(){}

    void SetSerializationMode(const SerializationModes& mode) { m_mode = mode; }
    const SerializationModes& GetSerializationMode() const { return m_mode; }
    uint32_t GetVersion() const { return m_version; }
    bool IsValid() const { return m_stream->device() && m_stream->device()->isOpen(); }

    bool IsReading() const { return m_isReading; }
    bool IsWriting() const { return !IsReading(); }
    QIODevice* GetDevice() const { return m_stream->device(); }
    Stream& GetStream() { return *m_stream; }

    template<class T>
    SerializerBufferBase& operator<<(T& data);
    SerializerBufferBase& operator<<(const PlainData& data);

    StandardVariantPropertiesContainer Properties;

protected:
    ScopedPointer<Stream> m_stream;
    uint32_t m_version;
    SerializationModes m_mode;
    bool m_isReading;
};

#pragma pack(1)
struct SerializerVersion
{
    qint64 Format = -1;
    qint32 Version = -1;
    qint64 HashSum = -1;
    qint32 SupportFromVersion = -1;

    SerializerVersion()
    {}

    SerializerVersion(qint64 format, qint32 version, qint32 supportVersionFrom)
        : Format(format)
        , Version(version)
        , SupportFromVersion(supportVersionFrom)
    {}

    QVariant CheckVersion(const SerializerVersion& current, qint64 size) const;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << HashSum;
        buffer << Format;
        buffer << Version;
    }
};
#pragma pack()

template<class Stream>
class SerializerHashSumWriter
{
    SerializerHashSumWriter(Stream* stream)
        : m_stream(stream)
        , m_pos(stream->GetDevice()->pos())
    {}

public:
    void WriteHashSum(qint64 hashSum)
    {
        auto currentPos = m_stream->GetDevice()->pos();
        m_stream->GetDevice()->seek(m_pos);
        *m_stream << hashSum;
        m_stream->GetDevice()->seek(currentPos);
    }

private:
    template<class T> friend class TSerializerWriteBuffer;
    Stream* m_stream;
    qint64 m_pos;
};

template<class Stream>
class TSerializerWriteBuffer : public SerializerBufferBase<Stream>
{
    using Super = SerializerBufferBase<Stream>;
public:
    TSerializerWriteBuffer(QIODevice* device)
        : Super(new Stream(device), false)
    {}

    TSerializerWriteBuffer(QByteArray* array, QIODevice::OpenMode openMode)
        : Super(new Stream(array, openMode), false)
    {}

    ~TSerializerWriteBuffer()
    {
    }

    SerializerHashSumWriter<TSerializerWriteBuffer> WriteVersion(const SerializerVersion& version)
    {
        Q_ASSERT(Super::IsValid());
        SerializerHashSumWriter<TSerializerWriteBuffer> result(this);
        *this << version;
        Super::m_version = version.Version;
        return result;
    }

    template<class T>
    TSerializerWriteBuffer& operator<<(const T& data)
    {
        Serializer<T>::Write(*this, data);
        return *this;
    }

    TSerializerWriteBuffer& operator<<(const PlainData& data)
    {
        Serializer<PlainData>::Write(*this, *const_cast<PlainData*>(&data));
        return *this;
    }
};

template<class Stream>
class TSerializerReadBuffer : public SerializerBufferBase<Stream>
{
    using Super = SerializerBufferBase<Stream>;
public:
    TSerializerReadBuffer(QIODevice* device)
        : Super(new Stream(device), true)
    {}

    TSerializerReadBuffer(const QByteArray& array)
        : Super(new Stream(array), true)
    {}

    SerializerVersion ReadVersion()
    {
        SerializerVersion result;
        if(Super::GetDevice() == nullptr) {
            return result;
        }
        if(Super::GetDevice()->bytesAvailable() < sizeof(SerializerVersion)) {
            return result;
        }
        *this << result;
        Super::m_version = result.Version;
        return result;
    }

    template<class T>
    TSerializerReadBuffer& operator<<(T& data)
    {
        Serializer<T>::Read(*this, data);
        return *this;
    }

    TSerializerReadBuffer& operator<<(const PlainData& data)
    {
        Serializer<PlainData>::Read(*this, *const_cast<PlainData*>(&data));
        return *this;
    }
};

#endif // STREAMBUFFER_H
