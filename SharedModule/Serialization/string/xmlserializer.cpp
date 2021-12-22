#include "xmlserializer.h"

static const Name VersionPropertyName("Version");
static const Name FormatPropertyName("Format");
static const Name CategoryPropertyName("Category");

QString SerializerXmlVersion::DataContainer::ToString(const QChar& separator) const
{
    QString result;
    for(auto it(begin()), e(end()); it != e; ++it) {
        result += QString("%1=%2%3").arg(it.key().AsString(), it.value().AsString(), separator);
    }
    return result;
}

void SerializerXmlVersion::DataContainer::FromString(const QString& string, const QChar& separator)
{
    clear();
    QRegExp regExp(QString(R"(\s?([^\=]+)\=([^%1]+)%1)").arg(separator));
    qint32 pos = 0;
    while((pos = regExp.indexIn(string, pos)) != -1) {
        insert(Name(regExp.cap(1)), Name(regExp.cap(2)));
        pos += regExp.matchedLength();
    }
}

void SerializerXmlVersion::SetVersion(qint32 version)
{
    Data.insert(VersionPropertyName, Name(QString::number(version)));
}

bool SerializerXmlVersion::HasVersion() const
{
    return Data.contains(VersionPropertyName);
}

qint32 SerializerXmlVersion::GetVersion() const
{
    auto value = Data.value(VersionPropertyName, Name());
    return value.IsNull() ? -1 : value.AsString().toInt();
}


void SerializerXmlVersion::SetFormat(qint64 format)
{
    Data.insert(FormatPropertyName, Name(QString::number(format)));
}

qint64 SerializerXmlVersion::GetFormat() const
{
    auto value = Data.value(FormatPropertyName, Name());
    return value.IsNull() ? -1 : value.AsString().toLongLong();
}
