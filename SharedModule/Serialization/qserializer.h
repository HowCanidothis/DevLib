#ifndef Q_SERIALIZER_H
#define Q_SERIALIZER_H

#include <QString>
#include <QDateTime>
#include <QVector>

#include <QDataStream>

#include "streambuffer.h"

class QDataStreamWriter : public QDataStream
{
    typedef QDataStream Super;
public:
    using Super::Super;

    void write(const char* data, size_t len) { writeRawData(data, (qint32)len); }

    bool good() const { return device() && device()->isOpen(); }
};

class QDataStreamReader : public QDataStreamWriter
{
    typedef QDataStreamWriter Super;
public:
    using Super::Super;

    void read(char* bytes, size_t len) { readRawData(bytes, (qint32)len); }
};

template<>
struct Serializer<QString>
{
    typedef QString target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        Serializer<int32_t>::Write(buffer, static_cast<int32_t>(data.size()));
        Serializer<PlainData>::Write(buffer, PlainData(data.constData(), data.size() * sizeof(target_type::value_type)));
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        int32_t size;
        Serializer<int32_t>::Read(buffer, size);
        data.resize(size);
        buffer << PlainData(data.data(), data.size() * sizeof(target_type::value_type));
    }
};

template<>
struct Serializer<QDate>
{
    using TypeName = QDate;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        auto julianDate = data.toJulianDay();
        buffer << julianDate;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        qint64 julianDate;
        buffer << julianDate;
        data = QDate::fromJulianDay(julianDate);
    }
};

template<>
struct Serializer<QSize>
{
    typedef QSize target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        auto w = data.width();
        auto h = data.height();
        buffer << w;
        buffer << h;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        buffer << data.rwidth();
        buffer << data.rheight();
    }
};

template<>
struct Serializer<QImage>
{
    typedef QImage target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer.GetStream() << data;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        buffer.GetStream() >> data;
    }
};

template<class T, class T2>
struct Serializer<QMap<T, T2>>
{
    typedef QMap<T, T2> target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        qint32 count = data.size();
        buffer << buffer.Attr("Size", count);
        for(auto it(data.begin()), e(data.end()); it != e; it++) {
            buffer << buffer.Sect("key", const_cast<T&>(it.key()));
            buffer << buffer.Sect("value", const_cast<T2&>(it.value()));
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        qint32 count = data.size();
        buffer << buffer.Attr("Size", count);
        data.clear();
        while(count--) {
            T key; T2 value;
            buffer << buffer.Sect("key", key);
            buffer << buffer.Sect("value", value);
            data.insert(key, value);
        }
    }
};

template<class T>
struct Serializer<QSet<T>>
{
    typedef QSet<T> target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        qint32 count = data.size();
        buffer << buffer.Attr("Size", count);
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_Sorted_Containers)) {
            auto list = data.toList();
            std::sort(list.begin(), list.end());
            for(const auto& element : list) {
                buffer << buffer.Sect("element", const_cast<T&>(element));
            }
        } else {
            for(const auto& element : data) {
                buffer << buffer.Sect("element", const_cast<T&>(element));
            }
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        qint32 count = data.size();
        buffer << buffer.Attr("Size", count);
        data.clear();
        if(count) {
            data.reserve(count);
            while(count--) {
                T element;
                buffer << buffer.Sect("element", element);
                data.insert(element);
            }
        }
    }
};

template<>
struct Serializer<QDateTime>
{
    typedef QDateTime Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        static qint64 invalidValue = -1;
        if(!type.isValid()) {
            buffer << invalidValue;
        } else {
            qint64 julianDay = type.toMSecsSinceEpoch();
            buffer << julianDay;
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint64 julianDay;
        buffer << julianDay;
        if(julianDay == -1) {
            type = QDateTime();
        } else {
            type = QDateTime::fromMSecsSinceEpoch(julianDay);
        }
    }
};

template<class T>
struct Serializer<QVector<T>>
{
    typedef QVector<T> Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        qint32 size = type.size();
        buffer << size;
        for(const T& value : type) {
            buffer << value;
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint32 size;
        buffer << size;
        type.resize(size);
        for(T& value : type) {
            buffer << value;
        }
    }
};

template<>
struct Serializer<QByteArray>
{
    typedef QByteArray Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        qint32 size = type.size();
        buffer << size;
        buffer << PlainData(type.data(), size);
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint32 size;
        buffer << size;
        type.resize(size);
        buffer << PlainData(type.data(), size);
    }
};

template<>
struct Serializer<QStringList>
{
    typedef QStringList Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        qint32 size = type.size();
        buffer << size;
        for(const auto& string : type) {
            buffer << string;
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint32 size;
        buffer << size;
        while(size--) {
            QString value;
            buffer << value;
            type.append(value);
        }
    }
};

template<typename Enum>
struct Serializer<QFlags<Enum>>
{
    typedef QFlags<Enum> Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        buffer << reinterpret_cast<const qint32&>(type);
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        buffer << reinterpret_cast<qint32&>(type);
    }
};

template<>
struct SerializerDirectionHelper<QDataStreamWriter>
{
    typedef StreamBufferBase<QDataStreamWriter> Buffer;
    template<typename T>
    static void Serialize(Buffer& buffer, const T& data) { Serializer<T>::Write(buffer, data); }
};

template<>
struct SerializerDirectionHelper<QDataStreamReader>
{
    typedef StreamBufferBase<QDataStreamReader> Buffer;
    template<typename T>
    static void Serialize(Buffer& buffer, T& data) { Serializer<T>::Read(buffer, data); }
};

typedef StreamBufferBase<QDataStreamWriter> QStreamBufferWrite;
typedef StreamBufferBase<QDataStreamReader> QStreamBufferRead;

template<class T>
inline QByteArray SerializeToArray(const T& object, SerializationModes serializationMode = SerializationMode_Default)
{
    QByteArray array;
    QStreamBufferWrite writer(&array, QIODevice::WriteOnly);
    writer.SetSerializationMode(serializationMode);
    writer << object;
    return array;
}

template<class T>
void DeSerializeFromArray(const QByteArray& array, T& object, SerializationModes serializationMode = SerializationMode_Default)
{
    QStreamBufferRead reader(array);
    reader.SetSerializationMode(serializationMode);
    reader << object;
}

#endif
