#include "name.h"
#include <QHashFunctions>

Name::Name()
    : _value(0)
{

}

Name::Name(const char* name)
    : _text(name)
    , _value(qHash(_text))

{

}

Name::Name(const Name& other)
    : _text(other._text)
    , _value(other._value)
{

}

Name::Name(const QString& name)
    : _text(name)
    , _value(qHash(_text))
{

}

void Name::SetName(const QString& str)
{
    _value = qHash(str);
    _text = str;
}

const QString& Name::AsString() const
{
    return _text;
}
