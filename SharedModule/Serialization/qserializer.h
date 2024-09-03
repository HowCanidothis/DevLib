#ifndef Q_SERIALIZER_H
#define Q_SERIALIZER_H

#include <QString>
#include <QDateTime>
#include <QVector>

#include <QDataStream>

#include <optional>

#include "streambuffer.h"

class QStreamBufferDataStream : public QDataStream
{
    typedef QDataStream Super;
public:
    using Super::Super;

    qint32 write(const char* data, size_t len) { return writeRawData(data, (qint32)len); }
    qint32 read(char* bytes, size_t len) { return readRawData(bytes, (qint32)len); }

    bool good() const { return device() && device()->isOpen(); }
};

using SerializerWriteBuffer = TSerializerWriteBuffer<QStreamBufferDataStream>;
using SerializerReadBuffer = TSerializerReadBuffer<QStreamBufferDataStream>;

struct DescSerializationWriteParams
{
    SerializationModes SerializationMode;
    std::function<void (SerializerWriteBuffer& )> InitHandler;

    DescSerializationWriteParams(qint32 mode)
        : SerializationMode((SerializationModes)mode)
        , InitHandler([](SerializerWriteBuffer&){})
    {}
    DescSerializationWriteParams(SerializationModes mode = SerializationMode_Default)
        : DescSerializationWriteParams((qint32)mode)
    {}

    DescSerializationWriteParams& SetInitHandler(const std::function<void (SerializerWriteBuffer& )>& handler)
    {
        InitHandler = handler;
        return *this;
    }
};

struct DescSerializationReadParams
{
    SerializationModes SerializationMode;
    std::function<bool (SerializerReadBuffer& )> InitHandler;

    DescSerializationReadParams(qint32 mode)
        : SerializationMode((SerializationModes)mode)
        , InitHandler([](SerializerReadBuffer& ){ return true; })
    {}

    DescSerializationReadParams(SerializationModes mode = SerializationMode_Default)
        : DescSerializationReadParams((qint32)mode)
    {}

    DescSerializationReadParams& SetInitHandler(const std::function<bool (SerializerReadBuffer& )>& handler)
    {
        InitHandler = handler;
        return *this;
    }
};

template<class T>
inline QByteArray SerializeToArray(const T& object, const DescSerializationWriteParams& params = DescSerializationWriteParams())
{
    QByteArray array;
    SerializerWriteBuffer writer(&array, QIODevice::WriteOnly);
    Q_ASSERT(params.InitHandler != nullptr);
    params.InitHandler(writer);
    writer.SetSerializationMode(params.SerializationMode);
    writer << object;
    return array;
}

template<class T>
bool DeSerializeFromArray(const QByteArray& array, T& object, const DescSerializationReadParams& params = DescSerializationReadParams())
{
    SerializerReadBuffer reader(array);
    Q_ASSERT(params.InitHandler != nullptr);
    if(!params.InitHandler(reader)) {
        return false;
    }
    reader.SetSerializationMode(params.SerializationMode);
    reader << object;
    return true;
}

template<class T>
inline void SerializeCopyObject(const T& source, T& target, SerializationMode mode)
{
    auto array = SerializeToArray(source, mode);
    DeSerializeFromArray(array, target, mode);
}

template<class T>
inline void SerializeCopyObject(const T& source, T& target, qint32 mode)
{
    SerializeCopyObject(source, target, (SerializationMode)mode);
}

template<class T>
inline QByteArray SerializeToArrayVersioned(const SerializerVersion& version, const T& object, DescSerializationWriteParams params = DescSerializationWriteParams())
{
    params.InitHandler = [&](SerializerWriteBuffer& buffer){
        buffer.WriteVersion(version);
    };
    return SerializeToArray(object, params);
}

template<class T>
std::pair<bool, SerializerVersion> DeSerializeFromArrayVersioned(const SerializerVersion& version, const QByteArray& array, T& object, DescSerializationReadParams params = DescSerializationReadParams())
{
    SerializerVersion currentVersion;
    params.InitHandler = [&](SerializerReadBuffer& buffer){
        currentVersion = buffer.ReadVersion();
        auto result = version.CheckVersion(currentVersion, buffer.GetDevice()->size());
        if(result.isValid()) {
            return false;
        }
        return true;
    };
    return std::make_pair(DeSerializeFromArray(array, object, params), currentVersion);
}

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
struct Serializer<QColor>
{
    using TypeName = QColor;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        qint32 r,g,b,a;
        r = data.red();
        g = data.green();
        b = data.blue();
        a = data.alpha();

        buffer << buffer.Sect("R", r);
        buffer << buffer.Sect("G", g);
        buffer << buffer.Sect("B", b);
        buffer << buffer.Sect("A", a);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        qint32 r,g,b,a;
        buffer << buffer.Sect("R", r);
        buffer << buffer.Sect("G", g);
        buffer << buffer.Sect("B", b);
        buffer << buffer.Sect("A", a);

        data.setRed(r);
        data.setGreen(g);
        data.setBlue(b);
        data.setAlpha(a);
    }
};

template<>
struct Serializer<QPointF>
{
    using TypeName = QPointF;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        auto x = data.x();
        auto y = data.y();
        buffer << buffer.Sect("X", x);
        buffer << buffer.Sect("Y", y);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        buffer << buffer.Sect("X", data.rx());
        buffer << buffer.Sect("Y", data.ry());
    }
};

template<>
struct Serializer<QRectF>
{
    using TypeName = QRectF;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        auto topLeft = data.topLeft();
        auto br = data.bottomRight();
        buffer << buffer.Sect("TopLeft", topLeft);
        buffer << buffer.Sect("BottomRight", br);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        QPointF topLeft, br;
        buffer << buffer.Sect("TopLeft", topLeft);
        buffer << buffer.Sect("BottomRight", br);
        data = QRectF(topLeft, br);
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

template<>
struct Serializer<QVariant>
{
    typedef QVariant target_type;
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

template<>
struct Serializer<QMatrix4x4>
{
    typedef QMatrix4x4 target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << PlainData(data.data(), sizeof(float) * 16);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        float values[16];
        buffer << PlainData(values, sizeof(values));
        data = QMatrix4x4(values);
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
struct Serializer<std::optional<T>>
{
    typedef std::optional<T> target_type;
    using value_type = T;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& constData)
    {
        bool hasValue = constData.has_value();
        buffer << buffer.Attr("IsValid", hasValue);
        if(hasValue) {
            auto& value = const_cast<value_type&>(constData.value());
            buffer << buffer.Attr("Value", value);
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        bool valid;
        buffer << buffer.Attr("IsValid", valid);
        if(valid) {
            value_type value;
            buffer << buffer.Attr("Value", value);
            data = value;
        } else {
            data = std::nullopt;
        }
    }
};

template<class T, class T2>
struct Serializer<QHash<T, T2>>
{
    typedef QHash<T, T2> target_type;
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
        if(!buffer.GetSerializationMode().TestFlag(SerializationMode_Merge_Containers)) {
            data.clear();
        }
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

template<>
struct Serializer<QTime>
{
    typedef QTime Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        static qint32 invalidValue = -1;
        if(!type.isValid()) {
            buffer << invalidValue;
        } else {
            qint32 msecs = type.msecsSinceStartOfDay();
            buffer << msecs;
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint32 msecs;
        buffer << msecs;
        if(msecs == -1) {
            type = QTime();
        } else {
            type = QTime::fromMSecsSinceStartOfDay(msecs);
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
        buffer << buffer.Attr("Size", size);
        for(const T& value : type) {
            buffer << buffer.Sect("element", const_cast<T&>(value));
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint32 size;
        buffer << buffer.Attr("Size", size);
        type.clear();
        type.resize(size);
        for(T& value : type) {
            buffer << buffer.Sect("element", value);
        }
    }
};

template<class T>
struct Serializer<QList<T>>
{
    typedef QList<T> Type;
    template<class Buffer>
    static void Write(Buffer& buffer, const Type& type)
    {
        qint32 size = type.size();
        buffer << buffer.Attr("Size", size);
        for(const T& value : type) {
            buffer << buffer.Sect("element", const_cast<T&>(value));
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Type& type)
    {
        qint32 size;
        buffer << buffer.Attr("Size", size);
        type.clear();
        while(size--) {
            T value;
            buffer << buffer.Sect("element", value);
            type.append(value);
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

#endif
