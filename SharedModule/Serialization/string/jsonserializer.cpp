#include "jsonserializer.h"

SerializerJsonWriteBuffer::SerializerJsonWriteBuffer(QJsonObject* writer)
    : Super(false)
    , m_currentContext(&m_context)
{
    m_currentObjects.append(ContextObject(writer));
}

void SerializerJsonWriteBuffer::BeginArrayObject()
{
    OpenSection(QString());
}

void SerializerJsonWriteBuffer::EndArrayObject()
{
    if(m_currentObjects.size() < 2) {
        return;
    }
    auto toAppend = m_currentObjects.takeLast();
    auto& last = m_currentObjects.last();
    last.Value.Add(QString(), toAppend.Value.ToValue());
}

void SerializerJsonWriteBuffer::WriteVersion(const SerializerXmlVersion& versionObject, const QChar& separator)
{
    m_version = versionObject.GetVersion();
    QJsonObject jsonVersionObject;
    jsonVersionObject.insert("target", versionObject.Target.AsString());
    jsonVersionObject.insert("data", versionObject.Data.ToString(separator));
    m_currentObjects.first().Value.Add("version", jsonVersionObject);
}

void SerializerJsonWriteBuffer::SetTextConverterContext(const TextConverterContext& context)
{
    m_context = context;
}

SerializerJsonWriteBuffer::ContextObject::ContextObject(QJsonObject* o)
    : Value(o)
{

}

SerializerJsonWriteBuffer::ContextObject::ContextObject(const QString& key, QJsonValue::Type type)
    : Key(key)
    , Value(type)
{
}

void SerializerJsonWriteBuffer::ContextObject::ResetWriterToArray()
{
    Value = SerializerJsonWriteContainer(QJsonValue::Array);
}

void SerializerJsonWriteBuffer::ContextObject::ResetWriterToObject()
{
    Value = SerializerJsonWriteContainer(QJsonValue::Object);
}

void SerializerJsonWriteBuffer::OpenSection(const QString& name)
{
    if(name.isEmpty() && m_currentObjects.size() == 1) {
        return;
    }
    m_currentObjects.append(ContextObject(name, QJsonValue::Object));
}

void SerializerJsonWriteBuffer::CloseSection()
{
    if(m_currentObjects.isEmpty()) {
        return;
    }
    auto last = m_currentObjects.takeLast();

    if(!m_currentObjects.isEmpty()) {
        auto& currentLast = m_currentObjects.last();
        currentLast.Value.Add(last.Key, last.Value.ToValue());
    }
}

SerializerJsonReadBuffer::SerializerJsonReadBuffer(const QJsonObject* reader)
    : Super(true)
{
    m_currentObjects.append(Context(*reader));
}

SerializerXmlVersion SerializerJsonReadBuffer::ReadVersion(const QChar& separator)
{
    SerializerXmlVersion result;

    auto foundIt = toObject().constFind("version");
    if(foundIt != toObject().constEnd()) {
        auto vo = foundIt->toObject();
        result.Data.FromString(vo.value("data").toString());
        result.Target = Name(vo.value("target").toString());
        auto hasVersion = result.HasVersion();
        if(hasVersion) {
            m_version = result.GetVersion();
            return result;
        }
    }
    return result;
}

const QJsonObject SerializerJsonReadBuffer::toObject() const
{
    if(m_currentObjects.isEmpty()) {
        return Default<QJsonObject>::Value;
    }
    return m_currentObjects.constLast().Value.toObject();
}

const QJsonArray SerializerJsonReadBuffer::toArray() const
{
    if(m_currentObjects.isEmpty()) {
        return Default<QJsonArray>::Value;
    }
    return m_currentObjects.constLast().Value.toArray();
}

void SerializerJsonReadBuffer::OpenSection(const QString& sectionName)
{
    if(sectionName.isEmpty() && m_currentObjects.size() == 1) {
        return;
    }

    const auto& v = m_currentObjects.constLast();
    if(v.Value.isArray()) {
        m_currentObjects.append(v.Value.toArray().at(v.CurrentIndex++));
    } else {
        m_currentObjects.append(v.Value.toObject().value(sectionName));
    }
}

SerializerJsonReadBuffer::Context::Context()
    : CurrentIndex(0)
    , ReadKeyMode(false)
{

}

SerializerJsonReadBuffer::Context::Context(const QJsonValue& v)
    : Value(v)
    , CurrentIndex(0)
    , ReadKeyMode(false)
{

}

IMPLEMENT_DEFAULT(QJsonObject);
IMPLEMENT_DEFAULT(QJsonArray);

QJsonValue SerializerJsonReadBuffer::Context::Read(const QString& key) const
{
    if(ReadKeyMode) {
        return (Value.toObject().begin() + CurrentIndex++).key();
    }

    if(Value.isObject()) {
        return Value.toObject().value(key);
    }
    auto ce = Value.toArray().at(CurrentIndex++);
    if(ce.isObject()) {
        return ce.toObject().value(key);
    }
    return ce;
}

void SerializerJsonReadBuffer::CloseSection()
{
    if(m_currentObjects.isEmpty()) {
        return;
    }
    m_currentObjects.removeLast();
}

void SerializerJsonReadBuffer::BeginArrayObject()
{
    auto& last = m_currentObjects.constLast();
    m_currentObjects.append(last.Value.toArray().at(last.CurrentIndex));
}

void SerializerJsonReadBuffer::EndArrayObject()
{
    m_currentObjects.pop_back();
    auto& last = m_currentObjects.last();
    last.CurrentIndex += 1;
}

SerializerJsonWriteContainer::SerializerJsonWriteContainer()
    : m_addHandler([](const QString&, const QJsonValue&){})
    , m_toValueHandler([]{ return QJsonValue(); })
{

}

SerializerJsonWriteContainer::SerializerJsonWriteContainer(QJsonObject* o)
{
    initObject(o);
}

SerializerJsonWriteContainer::SerializerJsonWriteContainer(QJsonArray* a)
{
    initArray(a);
}

SerializerJsonWriteContainer::SerializerJsonWriteContainer(QJsonValue::Type type)
{
    if(type == QJsonValue::Array) {
        m_allocatedArray = ::make_shared<QJsonArray>();
        initArray(m_allocatedArray.get());
    } else {
        m_allocatedObject = ::make_shared<QJsonObject>();
        initObject(m_allocatedObject.get());
    }
}

void SerializerJsonWriteContainer::Add(const QString& key, const QJsonValue& value)
{
    m_addHandler(key, value);
}

QJsonValue SerializerJsonWriteContainer::ToValue() const
{
    return m_toValueHandler();
}

void SerializerJsonWriteContainer::initArray(QJsonArray* a)
{
    m_array = a;
    m_addHandler = [a](const QString&, const QJsonValue& value) {
        a->append(value);
    };
    m_toValueHandler = [a] {
        return QJsonValue(*a);
    };
}

void SerializerJsonWriteContainer::initObject(QJsonObject* a)
{
    m_object = a;
    m_addHandler = [a](const QString& key, const QJsonValue& value) {
        a->insert(key, value);
    };
    m_toValueHandler = [a] {
        return QJsonValue(*a);
    };
}
