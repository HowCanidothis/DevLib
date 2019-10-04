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
{

}

Latin1Name::Latin1Name(const char* name)
    : m_text(name)
    , m_value(qHash(name))
{

}

void Latin1Name::SetName(const QLatin1String& str)
{
    m_value = qHash(str);
    m_text = str;
}

Latin1Name::Latin1Name(const QLatin1String& name)
    : m_text(name)
    , m_value(qHash(name))
{

}
