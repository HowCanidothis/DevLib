#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "stdserializer.h"

template<class Stream>
class StreamBufferBase
{
    bool _isValid;
    Stream _stream;
    int32_t _version;

public:
    template<class ... Args>
    StreamBufferBase(int64_t magicKey, int32_t version, Args ... args)
        : _stream(args...)
        , _version(version)
    {
        int64_t testKey = magicKey;
        if(_stream.good()) {
            *this << testKey;
            _isValid = (testKey == magicKey);
            if(_isValid) {
                *this << _version;
            }
        } else {
            _isValid = false;
        }
    }

    template<class Enum>
    StreamBufferBase(QByteArray* array, Enum flags)
        : _stream(array, flags)
        , _version(-1)
    {
        _isValid = _stream.good();
    }

    StreamBufferBase(const QByteArray& array)
        : _stream(array)
        , _version(-1)
    {
        _isValid = _stream.good();
    }


    int32_t GetVersion() const { return _version; }
    bool IsValid() const { return _isValid; }

    Stream& GetStream() { return _stream; }

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
