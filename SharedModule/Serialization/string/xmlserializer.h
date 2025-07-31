#ifndef XMLSERIALIZER_H
#define XMLSERIALIZER_H

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QBuffer>

#include "SharedModule/declarations.h"
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
        reader.OpenSection(object.Name);
        object.Value.Serialize(reader);
        reader.CloseSection();
    }
    template<class Buffer>
    static void Write(Buffer& writer, const SerializerXmlObject<T>& object)
    {
        writer.OpenSection(object.Name);
        object.Value.Serialize(writer);
        writer.CloseSection();
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
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(size_t)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(double)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(float)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(bool)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QString)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QImage)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QByteArray)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(Name)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QDateTime)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QDate)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QTime)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QColor)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QMatrix4x4)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QLocale)
SERIALIZER_XML_DECLARE_SIMPLE_TYPE(QVariant)

SERIALIZER_XML_DECL_SMART_POINTER_SERIALIZER(SharedPointer)
SERIALIZER_XML_DECL_SMART_POINTER_SERIALIZER(SharedPointerInitialized)
SERIALIZER_XML_DECL_SMART_POINTER_SERIALIZER(ScopedPointer)

#define DECLARE_SERIALIZER_XML_TO_SERIALIZER(ObjectType) \
template<> \
struct SerializerXml<ObjectType> \
{ \
    using Type = ObjectType; \
    template<class Buffer> \
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Read(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Write(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
};

#define DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(ObjectType) \
template<class T> \
struct SerializerXml<ObjectType<T>> \
{ \
    using Type = ObjectType<T>; \
    template<class Buffer> \
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Read(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Write(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
};

#define DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER_2(ObjectType) \
template<class T, class T2> \
struct SerializerXml<ObjectType<T, T2>> \
{ \
    using Type = ObjectType<T, T2>; \
    template<class Buffer> \
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Read(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Write(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
};

#define DECLARE_SERIALIZER_XML_TO_SERIALIZER_T(ObjectType) \
template<class T> \
struct SerializerXml<ObjectType<T>> \
{ \
    using Type = ObjectType<T>; \
    template<class Buffer> \
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Read(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer.OpenSection(object.Name); \
        Serializer<Type>::Write(buffer, object.Value); \
        buffer.CloseSection(); \
    } \
};

DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER_2(QMap)
DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER_2(QHash)
DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(QSet)
DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(QVector)
DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(QList)
DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(std::optional)
DECLARE_SERIALIZER_XML_TO_SERIALIZER(QRectF)
DECLARE_SERIALIZER_XML_TO_SERIALIZER(QPointF)
DECLARE_SERIALIZER_XML_TO_SERIALIZER_T(SPNullable)

template<>
struct SerializerXml<Id::Id>
{
    using Type = Id::Id;

    template<class Buffer>
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        SerializerXml<Name>::Read(buffer, object.Mutate(static_cast<Name&>(object.Value)));
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        SerializerXml<Name>::Write(buffer, object.Mutate(static_cast<Name&>(object.Value)));
    }
};

class SerializerXmlBufferBase
{
public:
    SerializerXmlBufferBase(bool isReading)
        : m_mode(SerializationMode_Default)
        , m_version(-1)
        , m_isReading(isReading)
    {}

    StandardVariantPropertiesContainer Properties;

    void SetSerializationMode(const SerializationModes& mode) { m_mode = mode; }
    const SerializationModes& GetSerializationMode() const { return m_mode; }

    template<class Buffer>
    void BeginArray(Buffer& buffer, qint32& size)
    {
        buffer << buffer.Attr("size", size);
    }

    void BeginArrayObject(){}
    void EndArrayObject(){}
    template<class Buffer>
    void BeginKeyValueArray(Buffer& buffer, qint32& size)
    {
        BeginArray(buffer, size);
    }

    template<class Buffer, typename T, typename T2>
    void KeyValue(Buffer& buffer, T& key, T2& value)
    {
        buffer << buffer.Sect("key", key);
        buffer << buffer.Sect("value", value);
    }

    quint32 GetVersion() const { return m_version; }

    bool IsReading() const { return m_isReading; }
    bool IsWriting() const { return !IsReading(); }

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

    template<class T>
    std::pair<SerializerXmlObject<T>,TextConverterContext> SectWithContext(const QString& name, T& value, const TextConverterContext& context)
    {
        return std::make_pair(SerializerXmlObject<T>(name, value, SerializerValueType::Section), context);
    }


protected:
    SerializationModes m_mode;
    quint32 m_version;
    bool m_isReading;
};

struct SerializerXmlVersion
{
    class DataContainer : public QHash<Name, Name>
    {
        using Super = QHash<Name, Name>;
    public:
        using Super::Super;

        QString ToString(const QChar& separator = ';') const;
        void FromString(const QString& string, const QChar& separator = ';');
    };

    Name Target;
    DataContainer Data;
    qint32 SupportVersionFrom;

    void SetVersion(qint32 version);
    bool HasVersion() const;
    qint32 GetVersion() const;

    void SetFormat(qint64 format);
    qint64 GetFormat() const;

    SerializerXmlVersion()
    {}

    SerializerXmlVersion(const Name& target, const DataContainer& data, qint32 supportVersionFrom)
        : Target(target)
        , Data(data)
        , SupportVersionFrom(supportVersionFrom)
    {}

    SerializerXmlVersion(const Name& target, qint64 format, qint32 version, qint32 supportVersionFrom)
        : Target(target)
        , SupportVersionFrom(supportVersionFrom)
    {
        SetVersion(version);
        SetFormat(format);
    }

    QVariant CheckVersion(const SerializerXmlVersion& another) const;

    Q_DECLARE_TR_FUNCTIONS(SerializerXmlVersion)
};

class SerializerXmlWriteBuffer : public SerializerXmlBufferBase
{
    using Super = SerializerXmlBufferBase;
public:
    SerializerXmlWriteBuffer(QXmlStreamWriter* writer)
        : Super(false)
        , m_writer(writer)
        , m_currentContext(&m_context)
    {}

    void WriteVersion(const SerializerXmlVersion& versionObject, const QChar& separator = ';')
    {
        m_version = versionObject.GetVersion();
        m_writer->writeProcessingInstruction(versionObject.Target.AsString(), versionObject.Data.ToString(separator));
    }

    void SetTextConverterContext(const TextConverterContext& context)
    {
        m_context = context;
    }

    template<class T>
    void SerializeAtomic(const SerializerXmlObject<T>& object)
    {
        if(object.Type == SerializerValueType::Attribute) {
            m_writer->writeAttribute(object.Name, TextConverter<T>::ToText(object.Value, *m_currentContext));
        } else {
            m_writer->writeTextElement(object.Name, TextConverter<T>::ToText(object.Value, *m_currentContext));
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

    template<class T>
    void operator<<(const std::pair<SerializerXmlObject<T>, TextConverterContext>& attributeValueWithContext)
    {
        m_currentContext = &attributeValueWithContext.second;
        SerializerXml<T>::Write(*this, attributeValueWithContext.first);
        m_currentContext = &m_context;
    }

private:
    QXmlStreamWriter* m_writer;
    TextConverterContext m_context;
    const TextConverterContext* m_currentContext;
};

class SerializerXmlReadBuffer : public SerializerXmlBufferBase
{
    using Super = SerializerXmlBufferBase;
public:
    SerializerXmlReadBuffer(QXmlStreamReader* reader)
        : Super(true)
        , m_reader(reader)
    {}

    SerializerXmlVersion ReadVersion(const QChar& separator = ';')
    {
        SerializerXmlVersion result;
        while(!m_reader->atEnd()) {
            auto tokenType = m_reader->readNext();
            switch(tokenType)
            {
            case QXmlStreamReader::ProcessingInstruction: {
                result.Data.FromString(m_reader->processingInstructionData().toString(), separator);
                result.Target = Name(m_reader->processingInstructionTarget().toString());
                auto hasVersion = result.HasVersion();
                if(hasVersion) {
                    m_version = result.GetVersion();
                    return result;
                }
                break;
            }
            case QXmlStreamReader::StartDocument: break;
            default: return result;
            }
        }
        return result;
    }

    template<class T>
    void SerializeAtomic(const SerializerXmlObject<T>& object)
    {
        if(object.Type == SerializerValueType::Attribute) {
            bool found = false;
            for(const auto& attribute : m_reader->attributes()) {
                if(attribute.name() == object.Name) {
                    object.Value = TextConverter<T>::FromText(attribute.value().toString());
                    found = true;
                    break;
                }
            }
            if(!found) {
                throw Exception("Parsing failed");
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

    template<class T>
    void operator<<(const std::pair<SerializerXmlObject<T>, TextConverterContext>& attributeValueWithContext)
    {
        operator<<(attributeValueWithContext.first);
    }

private:
    QXmlStreamReader* m_reader;
};

struct DescSerializationXMLWriteParams
{
    using FInitHandler = std::function<void (SerializerXmlWriteBuffer&)>;

    bool AutoFormating = false;
    SerializationModes Mode;
    TextConverterContext Context;

    DescSerializationXMLWriteParams(qint32 mode)
        : Mode((SerializationMode)mode)
    {}
    DescSerializationXMLWriteParams(SerializationModes mode = SerializationMode_Default)
        : Mode(mode)
    {}

    DescSerializationXMLWriteParams& SetAutoFormating(bool autoFormating) { AutoFormating = autoFormating; return *this; }
    DescSerializationXMLWriteParams& SetTextConverterContext(const TextConverterContext& context) { Context = context; return *this; }
};

template<class T>
inline QByteArray SerializeToXML(const QString& startSection, const T& object, const DescSerializationXMLWriteParams& properties, const DescSerializationXMLWriteParams::FInitHandler& initHandler = nullptr)
{
    QByteArray array;
    QXmlStreamWriter writer(&array);
    writer.setAutoFormatting(properties.AutoFormating);
    SerializerXmlWriteBuffer buffer(&writer);
    if(initHandler != nullptr) {
        initHandler(buffer);
    }
    buffer.SetSerializationMode(properties.Mode);
    buffer.SetTextConverterContext(properties.Context);
    buffer << buffer.Sect(startSection, const_cast<T&>(object));
    return array;
}

template<class T>
inline QByteArray SerializeToXMLVersioned(const SerializerXmlVersion& version, const QString& startSection, const T& object, DescSerializationXMLWriteParams properties)
{
    return SerializeToXML(startSection, object, properties, [&version](SerializerXmlWriteBuffer& buffer){
        buffer.WriteVersion(version);
    });
}

struct DescSerializationXMLReadParams
{
    using FInitHandler = std::function<bool (SerializerXmlReadBuffer&)>;
    SerializationModes Mode;
    FInitHandler InitHandler;

    DescSerializationXMLReadParams(qint32 mode)
        : Mode((SerializationMode)mode)
    {}
    DescSerializationXMLReadParams(SerializationModes mode = SerializationMode_Default)
        : Mode(mode)
    {}

    DescSerializationXMLReadParams& SetInitHandler(const FInitHandler& initHandler) { InitHandler = initHandler; return *this; }
};

template<class T, class StringOrArray>
bool DeSerializeFromXML(const QString& name, const StringOrArray& array, T& object, const DescSerializationXMLReadParams& properties, const DescSerializationXMLReadParams::FInitHandler& initHandler = nullptr)
{
    QXmlStreamReader reader(array);
    SerializerXmlReadBuffer buffer(&reader);
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
bool DeSerializeFromXMLVersioned(const SerializerXmlVersion& currentVersion, const QString& name, const StringOrArray& array, T& object, DescSerializationXMLReadParams properties)
{
    return DeSerializeFromXML(name, array, object, properties, [&](SerializerXmlReadBuffer& buffer){
        auto version = buffer.ReadVersion();
        auto checkVersionError = currentVersion.CheckVersion(version);
        if(checkVersionError.isValid()) {
            return false;
        }
        return true;
    });
}

inline std::pair<bool, SerializerXmlVersion> DeSerializeFromXMLCheckVersion(const SerializerXmlVersion& version, const QByteArray& array)
{
    QXmlStreamReader reader(array);
    SerializerXmlReadBuffer buffer(&reader);
    auto currentVersion = buffer.ReadVersion();
    return std::make_pair(!version.CheckVersion(currentVersion).isValid(), currentVersion);
}

#define DECLARE_SERIALIZER_XML_TYPE_ALIAS(SourceType, TargetType) \
template<> \
struct SerializerXml<TargetType> \
{ \
    using Type = TargetType; \
    template<class Buffer> \
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer << object.Mutate(reinterpret_cast<SourceType&>(object.Value)); \
    } \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        buffer << object.Mutate(reinterpret_cast<SourceType&>(object.Value)); \
    } \
};

#endif // XMLSERIALIZER_H
