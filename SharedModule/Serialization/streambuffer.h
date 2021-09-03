#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "stdserializer.h"
#include "SharedModule/flags.h"
#include "SharedModule/name.h"

static QVariant StreamBufferBaseDefaultPropertyResult;

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
        return StreamBufferBaseDefaultPropertyResult;
    }
};

template<class Stream>
class StreamBufferBase
{
public:
    template<class ... Args>
    StreamBufferBase(int32_t version, Args ... args)
        : m_stream(args...)
        , m_version(version)
        , m_mode(SerializationMode_Default)
    {
    }

    template<class Enum>
    StreamBufferBase(QByteArray* array, Enum flags)
        : m_stream(array, flags)
        , m_version(-1)
        , m_mode(SerializationMode_Default)
    {
        m_isValid = m_stream.good();
    }

    StreamBufferBase(const QByteArray& array)
        : m_stream(array)
        , m_version(-1)
        , m_mode(SerializationMode_Default)
    {
        m_isValid = m_stream.good();
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
    int32_t GetVersion() const { return m_version; }
    bool IsValid() const { return m_isValid; }
    bool IsGood() const { return m_stream.good(); }

    Stream& GetStream() { return m_stream; }

    template<class T>
    StreamBufferBase& operator<<(T& data);
    StreamBufferBase& operator<<(const PlainData& data);

    StandardVariantPropertiesContainer Properties;

protected:
    bool m_isValid;
    Stream m_stream;
    int32_t m_version;
    SerializationModes m_mode;
};

template<class Stream>
class StreamBufferWriter : public StreamBufferBase<Stream>
{
    using Super = StreamBufferBase<Stream>;
public:
    template<class ... Args>
    StreamBufferWriter(int64_t magicKey, int32_t version, Args ... args)
        : Super(version, args...)
        , m_magicKey(magicKey)
        , m_isFinished(false)
    {
        int64_t testKey = magicKey;
        if(Super::m_stream.good()) {
            *this << testKey;
            *this << Super::m_version;
        } else {
            Super::m_isValid = false;
        }
    }

    template<class Enum>
    StreamBufferWriter(QByteArray* array, Enum flags)
        : Super(array, flags)
        , m_magicKey(-1)
        , m_isFinished(true)
    {}

    ~StreamBufferWriter()
    {
        Q_ASSERT(m_isFinished);
    }

    void Finish()
    {
        Q_ASSERT(!m_isFinished);
        *this << m_magicKey;
        m_isFinished = true;
    }

    template<class T>
    StreamBufferWriter& operator<<(const T& data)
    {
        Serializer<T>::Write(*this, data);
        return *this;
    }

    StreamBufferWriter& operator<<(const PlainData& data)
    {
        Serializer<PlainData>::Write(*this, *const_cast<PlainData*>(&data));
        return *this;
    }

private:
    int64_t m_magicKey;
    bool m_isFinished;
};

template<class Stream>
class StreamBufferReader : public StreamBufferBase<Stream>
{
    using Super = StreamBufferBase<Stream>;
public:
    template<class ... Args>
    StreamBufferReader(int64_t magicKey, int32_t version, Args ... args)
        : Super(version, args...)
    {
        int64_t testKey;
        if(Super::m_stream.good()) {
            *this << testKey;
            Super::m_isValid = (testKey == magicKey);
            if(Super::m_isValid) {
                *this << Super::m_version;
                Super::m_isValid = Super::m_stream.GetLastInt64() == magicKey;
            }            
        } else {
            Super::m_isValid = false;
        }
    }

    StreamBufferReader(const QByteArray& array)
        : Super(array)
    {}

    template<class T>
    StreamBufferReader& operator<<(T& data)
    {
        Serializer<T>::Read(*this, data);
        return *this;
    }

    StreamBufferReader& operator<<(const PlainData& data)
    {
        Serializer<PlainData>::Read(*this, *const_cast<PlainData*>(&data));
        return *this;
    }

private:
    int64_t m_magicKey;
};

#endif // STREAMBUFFER_H
