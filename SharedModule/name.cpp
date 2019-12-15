#include "name.h"
#include <QHashFunctions>

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
    : m_value(0)
    , m_text(new std::string)
{

}

Latin1Name::Latin1Name(const char* name)
    : m_text(new std::string(name))
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
