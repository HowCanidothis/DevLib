#include "name.h"
#include <QHashFunctions>

UniName::UniName(const char* name)
    : m_name(name)
    , m_latinName(name)
{

}

UniName::UniName(const Latin1Name& name)
    : m_name(QString::fromStdString(name.AsLatin1String()))
    , m_latinName(name)
{

}

UniName::UniName(const Name& name)
    : m_name(name)
    , m_latinName(name.AsString().toStdString())
{

}

UniName::UniName(const std::string& name)
    : m_name(QString::fromStdString(name))
    , m_latinName(name)
{

}

UniName::UniName(std::string&& name)
    : m_name(QString::fromStdString(name))
    , m_latinName(std::move(name))
{

}

UniName::UniName(const QString& name)
    : m_name(name)
    , m_latinName(name.toStdString())
{

}

void UniName::SetName(const std::string& name)
{
    m_name.SetName(QString::fromStdString(name));
    m_latinName.SetName(name);
}

void UniName::SetName(const QString& name)
{
    m_name.SetName(name);
    m_latinName.SetName(name.toStdString());
}

Name::Name()
    : m_value(0)
{

}

Name::Name(const char* name)
    : m_text(name)
    , m_value(qHash(m_text))

{

}

Name::Name(const Name& other)
    : m_text(other.m_text)
    , m_value(other.m_value)
{

}

Name::Name(const QString& name)
    : m_text(name)
    , m_value(qHash(m_text))
{

}

Name& Name::Join(const Name& o) {
    m_text += o.m_text;
    m_value = qHash(m_text);
    return *this;
}
Name Name::Join(const Name& o) const {
    Name ret(*this);
    ret.Join(o);
    return ret;
}

void Name::SetName(const QString& str)
{
    m_value = qHash(str);
    m_text = str;
}

const QString& Name::AsString() const
{
    return m_text;
}

Latin1Name::Latin1Name()
    : m_text(new std::string)
    , m_value(0)
{

}

Latin1Name::Latin1Name(const char* name)
    : m_text(new std::string(name))
    , m_value(qHashBits(m_text->data(), m_text->size()))
{

}

Latin1Name::Latin1Name(std::string&& name)
    : m_text(new std::string(std::move(name)))
    , m_value(qHashBits(m_text->data(), m_text->size()))
{

}

void Latin1Name::SetName(const std::string& name)
{
    m_value = qHashBits(name.data(), name.size());
    *m_text = name;
}

Latin1Name::Latin1Name(const std::string& name)
    : m_text(new std::string(name))
    , m_value(qHashBits(name.data(), name.size()))
{

}
