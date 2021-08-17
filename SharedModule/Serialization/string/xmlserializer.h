#ifndef XMLSERIALIZER_H
#define XMLSERIALIZER_H

#include <QXmlStreamReader>
#include <QBuffer>

#include "SharedModule/shared_decl.h"
#include "SharedModule/Serialization/qserializer.h"

enum class SerializerValueType
{
    Section,
    Attribute
};

template<class T>
struct SerializerXmlObject
{
    QString Name;
    T& Value;
    SerializerValueType Type;

    SerializerXmlObject(const QString& name, T& value, SerializerValueType type = SerializerValueType::Section)
        : Name(name)
        , Value(value)
        , Type(type)
    {}

    template<class T2>
    SerializerXmlObject<T2> Mutate(T2& value) const { return SerializerXmlObject<T2>(Name, value, Type); }
};

template<class T>
struct SerializerXml
{
    template<class Buffer>
    static void Read(Buffer& reader, const SerializerXmlObject<T>& object)
    {
        object.Value.Serialize(reader);
    }
    template<class Buffer>
    static void Write(Buffer& writer, const SerializerXmlObject<T>& object)
    {
        object.Value.Serialize(writer);
    }
};

#define SERIALIZER_XML_DECLARE_SIMPLE_TYPE(T) \
template<> \
struct SerializerXml<T> \
{ \
    template<class Buffer> \
    static void Read(Buffer& reader, const SerializerXmlObject<T>& object) \
    { \
        reader.SerializeAtomic(object); \
    } \
    template<class Buffer> \
    static void Write(Buffer& writer, const SerializerXmlObject<T>& object) \
    { \
        writer.SerializeAtomic(object); \
    } \
};

#define SERIALIZER_XML_DECL_SMART_POINTER_SERIALIZER(type) \
template<typename T> \
struct SerializerXml<type<T>> \
{ \
    typedef type<T> SmartPointer; \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<SmartPointer>& pointer) \
    { \
        buffer << pointer.Mutate(*pointer.Value); \
    } \
    template<class Buffer> \
    static void Read(Buffer& buffer, const SerializerXmlObject<SmartPointer>& pointer) \
    { \
        if(pointer.Value == nullptr) { \
            pointer.Value.reset(new T()); \
        } \
        buffer << pointer.Mutate(*pointer.Value); \
    } \
};

SERIALIZER_XML_DECLARE_SIMPLE_TYPE(qint64)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(qint32)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(double)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(float)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(bool)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QString)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QByteArray)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(Name)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QDateTime)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QDate)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QColor)

SERIALIZER_XML_DECL_SMART_POINTER_SERIALIZER(SharedPointer)
SERIALIZER_XML_DECL_SMART_POINTER_SERIALIZER(ScopedPointer)

template<class T>
struct SerializerXml<QVector<T>>
{
    using Type = QVector<T>;

    template<class Buffer>
    static void Read(Buffer& reader, const SerializerXmlObject<Type>& object)
    {
        reader.OpenSection(object.Name);
        qint32 count = 0;
        reader << reader.Attr("Size", count);
        object.Value.clear();
        while(count--) {
            T element;
            reader << reader.Sect("element", element);
            object.Value.append(element);
        }
        reader.CloseSection();
    }

    template<class Buffer>
    static void Write(Buffer& writer, const SerializerXmlObject<Type>& object)
    {
        writer.OpenSection(object.Name);
        qint32 count = object.Value.size();
        writer << writer.Attr("Size", count);
        for(auto& element : object.Value) {
            writer << writer.Sect("element", element);
        }
        writer.CloseSection();
    }
};

template<class T>
struct SerializerXml<QSet<T>>
{
    using Type = QSet<T>;

    template<class Buffer>
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer.OpenSection(object.Name);
        Serializer<Type>::Read(buffer, object.Value);
        buffer.CloseSection();
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer.OpenSection(object.Name);
        Serializer<Type>::Write(buffer, object.Value);
        buffer.CloseSection();
    }
};

template<>
struct SerializerXml<QImage>
{
    using Type = QImage;

    template<class Buffer>
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer.OpenSection(object.Name);
        QByteArray array;
        buffer << buffer.Attr("Data", array);
        object.Value.fromData(array);
        buffer.CloseSection();
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer.OpenSection(object.Name);
        QByteArray array;
        QBuffer arrayBuffer(&array);
        arrayBuffer.open(QIODevice::WriteOnly);
        object.Value.save(&arrayBuffer);
        buffer << buffer.Attr("Data", array);
        buffer.CloseSection();
    }
};

class SerializerXmlBufferBase
{
public:
    SerializerXmlBufferBase()
        : m_mode(SerializationMode_Default)
    {}

    StandardVariantPropertiesContainer Properties;

    void SetSerializationMode(const SerializationModes& mode) { m_mode = mode; }
    const SerializationModes& GetSerializationMode() const { return m_mode; }

    template<class T>
    SerializerXmlObject<T> Attr(const QString& name, T& value)
    {
        return SerializerXmlObject<T>(name, value, SerializerValueType::Attribute);
    }

    template<class T>
    SerializerXmlObject<T> Sect(const QString& name, T& value)
    {
        return SerializerXmlObject<T>(name, value, SerializerValueType::Section);
    }


private:
    SerializationModes m_mode;
};

class SerializerXmlWriteBuffer : public SerializerXmlBufferBase
{
public:
    SerializerXmlWriteBuffer(QXmlStreamWriter* writer)
        : m_writer(writer)
    {}

    void SetTextConverterContext(const TextConverterContext& context)
    {
        m_context = context;
    }

    template<class T>
    void SerializeAtomic(const SerializerXmlObject<T>& object)
    {
        if(object.Type == SerializerValueType::Attribute) {
            m_writer->writeAttribute(object.Name, TextConverter<T>::ToText(object.Value, m_context));
        } else {
            m_writer->writeTextElement(object.Name, TextConverter<T>::ToText(object.Value, m_context));
        }
    }

    void OpenSection(const QString& name)
    {
        m_writer->writeStartElement(name);
    }

    void CloseSection()
    {
        m_writer->writeEndElement();
    }

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

private:
    QXmlStreamWriter* m_writer;
    TextConverterContext m_context;
};

class SerializerXmlReadBuffer : public SerializerXmlBufferBase
{
public:
    SerializerXmlReadBuffer(QXmlStreamReader* reader)
        : m_reader(reader)
    {}

    template<class T>
    void SerializeAtomic(const SerializerXmlObject<T>& object)
    {
        if(object.Type == SerializerValueType::Attribute) {
            for(const auto& attribute : m_reader->attributes()) {
                if(attribute.name() == object.Name) {
                    object.Value = TextConverter<T>::FromText(attribute.value().toString());
                    break;
                }
            }
        } else {
            OpenSection("");
            object.Value = TextConverter<T>::FromText(m_reader->readElementText());
        }
    }

    QString CurrentText()
    {
        return m_reader->readElementText();
    }

    void OpenSection(const QString&)
    {
        while(!m_reader->readNextStartElement() && !m_reader->atEnd());
    }

    void CloseSection()
    {

    }

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

private:
    QXmlStreamReader* m_reader;
};

struct DescSerializationXMLWriteProperties
{
    bool AutoFormating = false;
    SerializationModes Mode = SerializationMode_Default;
    TextConverterContext Context;

    DescSerializationXMLWriteProperties& SetAutoFormating(bool autoFormating) { AutoFormating = autoFormating; return *this; }
    DescSerializationXMLWriteProperties& SetTextConverterContext(const TextConverterContext& context) { Context = context; return *this; }
    DescSerializationXMLWriteProperties& SetSerializationMode(SerializationModes serializationMode) { Mode = serializationMode; return *this; }
};

template<class T>
inline QByteArray SerializeToXML(const T& object, const DescSerializationXMLWriteProperties& properties)
{
    QByteArray array;
    QXmlStreamWriter writer(&array);
    writer.setAutoFormatting(properties.AutoFormating);
    SerializerXmlWriteBuffer buffer(&writer);
    buffer.SetSerializationMode(properties.Mode);
    buffer.SetTextConverterContext(properties.Context);
    buffer << const_cast<T&>(object);
    return array;
}

template<class T>
inline QByteArray SerializeToXML(const QString& startSection, const T& object, const DescSerializationXMLWriteProperties& properties)
{
    QByteArray array;
    QXmlStreamWriter writer(&array);
    writer.setAutoFormatting(properties.AutoFormating);
    SerializerXmlWriteBuffer buffer(&writer);
    buffer.SetSerializationMode(properties.Mode);
    buffer.SetTextConverterContext(properties.Context);
    buffer << buffer.Sect(startSection, const_cast<T&>(object));
    return array;
}

template<class T>
void DeSerializeFromXML(const QByteArray& array, T& object, SerializationModes serializationMode = SerializationMode_Default)
{
    QXmlStreamReader reader(array);
    SerializerXmlReadBuffer buffer(&reader);
    buffer.SetSerializationMode(serializationMode);
    buffer << object;
}

template<class T>
void DeSerializeFromXML(const QString& name, const QByteArray& array, T& object, SerializationModes serializationMode = SerializationMode_Default)
{
    QXmlStreamReader reader(array);
    SerializerXmlReadBuffer buffer(&reader);
    buffer.SetSerializationMode(serializationMode);
    buffer << buffer.Sect(name, object);
}

#endif // XMLSERIALIZER_H
