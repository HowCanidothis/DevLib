#include "name.h"
#include <QHash>

Name::Name(const char* name)
    : value(qHash(name))
    , transcription(name)
{

}

Name::Name(const QString& name)
    : value(qHash(name))
    , transcription(name)
{

}

const QString& Name::asString() const
{
    return transcription;
}
