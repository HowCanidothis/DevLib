#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "xmlserializer.h"

template<class T>
inline QJsonValue SerializerJsonWriteBufferToJsonValue(const T& v, const TextConverterContext& context)
{
    return v;
}

// TODO. QJsonValue supports int64 since Qt 6.0
template<>
inline QJsonValue SerializerJsonWriteBufferToJsonValue(const qint64& v, const TextConverterContext&)
{
    return QJsonValue(QString::number(v));
}

template<>
inline QJsonValue SerializerJsonWriteBufferToJsonValue(const size_t& v, const TextConverterContext& context)
{
    return SerializerJsonWriteBufferToJsonValue(qint64(v), context);
}

template<class T>
inline void SerializerJsonReadBufferFromJsonValue(T& v, const QJsonValue& jv)
{
    switch(jv.type()) {
    case QJsonValue::Bool: v = jv.toBool(); break;
    case QJsonValue::Double: v = jv.toDouble(); break;
    default: v = TextConverter<T>::FromText(jv.toString()); break;
    }
}

#define DECLARE_SERIALIZER_JSON_AS_STRING(type) \
template<> \
inline QJsonValue SerializerJsonWriteBufferToJsonValue(const type& v, const TextConverterContext& context) \
{\
    return TextConverter<type>::ToText(v, context); \
}\
template<> \
inline void SerializerJsonReadBufferFromJsonValue(type& v, const QJsonValue& jv) \
{ \
    v = TextConverter<type>::FromText(jv.toString()); \
}

DECLARE_SERIALIZER_JSON_AS_STRING(Name)
DECLARE_SERIALIZER_JSON_AS_STRING(QByteArray)
DECLARE_SERIALIZER_JSON_AS_STRING(QDateTime)
DECLARE_SERIALIZER_JSON_AS_STRING(QDate)
DECLARE_SERIALIZER_JSON_AS_STRING(QImage)
DECLARE_SERIALIZER_JSON_AS_STRING(QColor)
DECLARE_SERIALIZER_JSON_AS_STRING(QVariant)
DECLARE_SERIALIZER_JSON_AS_STRING(QTime)
DECLARE_SERIALIZER_JSON_AS_STRING(QMatrix4x4)
DECLARE_SERIALIZER_JSON_AS_STRING(QLocale)



class SerializerJsonWriteContainer
{
public:
    SerializerJsonWriteContainer();
    SerializerJsonWriteContainer(QJsonObject* o);
    SerializerJsonWriteContainer(QJsonArray* a);
    SerializerJsonWriteContainer(QJsonValue::Type type);

    void Add(const QString& key, const QJsonValue& value);

    QJsonValue ToValue() const;

private:
    void initArray(QJsonArray* a);

    void initObject(QJsonObject* a);

private:
    union {
        QJsonArray* m_array;
        QJsonObject* m_object;
    };
    std::function<void (const QString& key, const QJsonValue& value)> m_addHandler;
    std::function<QJsonValue ()> m_toValueHandler;

    SP<QJsonArray> m_allocatedArray;
    SP<QJsonObject> m_allocatedObject;
};

class SerializerJsonWriteBuffer : public SerializerXmlBufferBase
{
    using Super = SerializerXmlBufferBase;
public:
    using FInitHandler = std::function<void (SerializerJsonWriteBuffer&)>;

    SerializerJsonWriteBuffer(QJsonObject* writer);

    void WriteVersion(const SerializerXmlVersion& versionObject, const QChar& separator = ';');

    void SetTextConverterContext(const TextConverterContext& context);

    template<class T>
    void SerializeAtomic(const SerializerXmlObject<T>& object)
    {
        if(!m_currentObjects.isEmpty()) {
            auto& last = m_currentObjects.last();
            last.Value.Add(object.Name, SerializerJsonWriteBufferToJsonValue(object.Value, *m_currentContext));
        }
    }

    void OpenSection(const QString& name);
    template<class Buffer>
    void BeginArray(Buffer&, qint32&)
    {
        m_currentObjects.last().ResetWriterToArray();
    }

    void BeginArrayObject();
    void EndArrayObject();
    template<class Buffer>
    void BeginKeyValueArray(Buffer&, qint32&)
    {
    }

    template<class Buffer, typename T, typename T2>
    void KeyValue(Buffer& buffer, T& key, T2& value)
    {
        buffer << buffer.Sect(TextConverter<T>::ToText(key, TextConverterContext()), value);
    }

    void CloseSection();

    template<class T>
    void operator<<(T& object)
    {
        object.Serialize(*this);
    }

    template<class T>
    void operator<<(const SerializerXmlObject<T>& attributeValue)
    {
        SerializerXml<T>::Write(*this, attributeValue);
    }

    template<class T>
    void operator<<(const std::pair<SerializerXmlObject<T>, TextConverterContext>& attributeValueWithContext)
    {
        m_currentContext = &attributeValueWithContext.second;
        SerializerXml<T>::Write(*this, attributeValueWithContext.first);
        m_currentContext = &m_context;
    }

private:
    struct ContextObject
    {
        QString Key;
        SerializerJsonWriteContainer Value;

        ContextObject(QJsonObject* o);
        ContextObject(const QString& key, QJsonValue::Type type);

        void ResetWriterToObject();
        void ResetWriterToArray();
    };

    QVector<ContextObject> m_currentObjects;
    TextConverterContext m_context;
    const TextConverterContext* m_currentContext;
};

class SerializerJsonReadBuffer : public SerializerXmlBufferBase
{
    using Super = SerializerXmlBufferBase;
public:
    using FInitHandler = std::function<bool (SerializerJsonReadBuffer&)>;

    SerializerJsonReadBuffer(const QJsonObject* reader);

    SerializerXmlVersion ReadVersion(const QChar& separator = ';');

    template<class T>
    void SerializeAtomic(const SerializerXmlObject<T>& object)
    {
        const auto& last = m_currentObjects.constLast();
        auto jsonValue = last.Read(object.Name);


        if(jsonValue.isNull()) {
            return;
        }

        SerializerJsonReadBufferFromJsonValue(object.Value, jsonValue);
    }

    template<class Buffer>
    void BeginArray(Buffer&, qint32& size)
    {
        const auto& l = m_currentObjects.constLast().Value;
        if(l.isArray()) {
            size = l.toArray().size();
        } else {
            size = l.toObject().size();
        }
    }

    template<class Buffer>
    void BeginKeyValueArray(Buffer& buffer, qint32& size)
    {
        BeginArray(buffer, size);
    }

    template<class Buffer, typename T, typename T2>
    void KeyValue(Buffer& buffer, T& key, T2& value)
    {
        auto& last = m_currentObjects.constLast();
        last.ReadKeyMode = true;
        auto jsonValue = last.Read(QString());
        last.ReadKeyMode = false;
        if(jsonValue.isNull()) {
            return;
        }
        key = TextConverter<T>::FromText(jsonValue.toString());
        buffer << buffer.Sect(jsonValue.toString(), value);
    }

    void OpenSection(const QString& sectionName);
    void CloseSection();

    void BeginArrayObject();
    void EndArrayObject();

    template<class T>
    void operator<<(T& object)
    {
        object.Serialize(*this);
    }

    template<class T>
    void operator<<(const SerializerXmlObject<T>& attributeValue)
    {
        SerializerXml<T>::Read(*this, const_cast<SerializerXmlObject<T>&>(attributeValue));
    }

    template<class T>
    void operator<<(const std::pair<SerializerXmlObject<T>, TextConverterContext>& attributeValueWithContext)
    {
        operator<<(attributeValueWithContext.first);
    }

private:
    const QJsonObject toObject() const;
    const QJsonArray toArray() const;

    struct Context
    {
        QJsonValue Value;
        mutable qint32 CurrentIndex;
        mutable bool ReadKeyMode;

        Context();
        Context(const QJsonValue& v);

        QJsonValue Read(const QString& key) const;
    };

    QVector<Context> m_currentObjects;
};

template<class T>
inline QByteArray SerializeToJson(const QString& startSection, const T& object, const DescSerializationXMLWriteParams& properties, const SerializerJsonWriteBuffer::FInitHandler& initHandler = nullptr)
{
    QJsonDocument document;
    auto jsonObject = SerializeToJsonObject(startSection, object, properties, initHandler);
    document.setObject(jsonObject);
    return document.toJson(properties.AutoFormating ? QJsonDocument::Indented : QJsonDocument::Compact);
}

template<class T>
inline QJsonObject SerializeToJsonObject(const QString& startSection, const T& object, const DescSerializationXMLWriteParams& properties, const SerializerJsonWriteBuffer::FInitHandler& initHandler = nullptr)
{
    QJsonObject jsonObject;
    SerializerJsonWriteBuffer buffer(&jsonObject);
    if(initHandler != nullptr) {
        initHandler(buffer);
    }
    buffer.SetSerializationMode(properties.Mode);
    buffer.SetTextConverterContext(properties.Context);
    buffer << buffer.Sect(startSection, const_cast<T&>(object));
    return jsonObject;
}

template<class T>
inline QByteArray SerializeToJsonVersioned(const SerializerXmlVersion& version, const QString& startSection, const T& object, DescSerializationXMLWriteParams properties)
{
    return SerializeToJson(startSection, object, properties,[&version](SerializerJsonWriteBuffer& buffer){
        buffer.WriteVersion(version);
    });
}

template<class T, class StringOrArray>
bool DeSerializeFromJson(const QString& name, const StringOrArray& array, T& object, const DescSerializationXMLReadParams& properties, const SerializerJsonReadBuffer::FInitHandler& initHandler = nullptr)
{
    auto document = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = document.object();
    return DeSerializeFromJson(name, jsonObject, object, properties, initHandler);
}

template<class T>
bool DeSerializeFromJson(const QString& name, const QJsonObject& jsonObject, T& object, const DescSerializationXMLReadParams& properties, const SerializerJsonReadBuffer::FInitHandler& initHandler = nullptr)
{
    SerializerJsonReadBuffer buffer(&jsonObject);

    if(initHandler != nullptr) {
        if(!initHandler(buffer)) {
            return false;
        }
    }
    buffer.SetSerializationMode(properties.Mode);
    buffer << buffer.Sect(name, object);
    return true;
}

template<class T, class StringOrArray>
bool DeSerializeFromJsonVersioned(const SerializerXmlVersion& currentVersion, const QString& name, const StringOrArray& array, T& object, DescSerializationXMLReadParams properties)
{
    return DeSerializeFromJson(name, array, object, properties, [&](SerializerJsonReadBuffer& buffer){
        auto version = buffer.ReadVersion();
        auto checkVersionError = currentVersion.CheckVersion(version);
        if(checkVersionError.isValid()) {
            return false;
        }
        return true;
    });
}

inline std::pair<bool, SerializerXmlVersion> DeSerializeFromJsonCheckVersion(const SerializerXmlVersion& version, const QJsonObject& jsonObject)
{
    SerializerJsonReadBuffer buffer(&jsonObject);
    auto currentVersion = buffer.ReadVersion();
    return std::make_pair(!version.CheckVersion(currentVersion).isValid(), currentVersion);
}

#endif // JSONSERIALIZER_H
