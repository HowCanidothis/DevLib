#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "stdserializer.h"
#include "SharedModule/flags.h"

enum SerializationMode {
    SerializationMode_Default = 0x0,
    SerializationMode_InvokeProperties = 0x1,
    SerializationMode_MinMaxProperties = 0x2,
    SerializationMode_PrecisionProperties = 0x4,
    SerializationMode_UserDefined = 0x200
};
DECL_FLAGS(SerializationModes, SerializationMode);

template<class Stream>
class StreamBufferBase
{
    bool m_isValid;
    Stream m_stream;
    int32_t m_version;
    SerializationModes m_mode;

public:
    template<class ... Args>
    StreamBufferBase(int64_t magicKey, int32_t version, Args ... args)
        : m_stream(args...)
        , m_version(version)
        , m_mode(SerializationMode_Default)
    {
        int64_t testKey = magicKey;
        if(m_stream.good()) {
            *this << testKey;
            m_isValid = (testKey == magicKey);
            if(m_isValid) {
                *this << m_version;
            }
        } else {
            m_isValid = false;
        }
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

    void SetSerializationMode(const SerializationModes& mode) { m_mode = mode; }
    const SerializationModes& GetSerializationMode() const { return m_mode; }
    int32_t GetVersion() const { return m_version; }
    bool IsValid() const { return m_isValid; }
    bool IsGood() const { return m_stream.good(); }

    Stream& GetStream() { return m_stream; }

    template<class T>
    StreamBufferBase& operator<<(T& data);
    StreamBufferBase& operator<<(const PlainData& data);
};

template<class Stream>
struct SerializerDirectionHelper
{
    template<typename T>
    static void Serialize(StreamBufferBase<Stream>& buffer, const T&);
};

template<>
struct SerializerDirectionHelper<std::istream>
{
    typedef StreamBufferBase<std::istream> Buffer;
    template<typename T>
    static void Serialize(Buffer& buffer, T& data) { Serializer<T>::Read(buffer, data); }
};

template<>
struct SerializerDirectionHelper<std::ostream>
{
    typedef StreamBufferBase<std::ostream> Buffer;
    template<typename T>
    static void Serialize(Buffer& buffer, const T& data) { Serializer<T>::Write(buffer, data); }
};

template<>
struct SerializerDirectionHelper<std::ifstream>
{
    typedef StreamBufferBase<std::ifstream> Buffer;
    template<typename T>
    static void Serialize(Buffer& buffer, T& data) { Serializer<T>::Read(buffer, data); }
};

template<>
struct SerializerDirectionHelper<std::ofstream>
{
    typedef StreamBufferBase<std::ofstream> Buffer;
    template<typename T>
    static void Serialize(Buffer& buffer, const T& data) { Serializer<T>::Write(buffer, data); }
};

template<class Stream> template<class T>
StreamBufferBase<Stream>& StreamBufferBase<Stream>::operator<<(T& data)
{
    SerializerDirectionHelper<Stream>::Serialize(*this, data);
    return *this;
}

template<class Stream>
StreamBufferBase<Stream>& StreamBufferBase<Stream>::operator<<(const PlainData& data)
{
    SerializerDirectionHelper<Stream>::Serialize(*this, *const_cast<PlainData*>(&data));
    return *this;
}

typedef StreamBufferBase<std::istream> StreamBufferRead;
typedef StreamBufferBase<std::ostream> StreamBufferWrite;
typedef StreamBufferBase<std::ifstream> StreamBufferReadFile;
typedef StreamBufferBase<std::ofstream> StreamBufferWriteFile;

#endif // STREAMBUFFER_H
