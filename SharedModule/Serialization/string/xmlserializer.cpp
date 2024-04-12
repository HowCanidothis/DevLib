#include "xmlserializer.h"

#include <QRegularExpression>

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

QVariant SerializerXmlVersion::CheckVersion(const SerializerXmlVersion &another) const
{
    if(another.Target != Target) {
        return tr("Unexpected file contents");
    }
    if(another.GetFormat() != GetFormat()) {
        return tr("Format error - expected %1, but file version is %2").arg(QString::number(GetFormat()) , QString::number(another.GetFormat()));
    }
    auto currentVersionValue = (quint32)another.GetVersion();
    if(SupportVersionFrom != -1 && currentVersionValue < (quint32)SupportVersionFrom) {
        return tr("Version is not supported - application supports versions starting from %1, but file version is %2").arg(QString::number(SupportVersionFrom) , QString::number(another.GetVersion()));
    }
     if(currentVersionValue > (quint32)GetVersion()) {
        return tr("Future version error - application supported version is %1, but file version is %2").arg(QString::number(GetVersion()) , QString::number(another.GetVersion()));
    }

    return QVariant();
}

void SerializerXmlVersion::DataContainer::FromString(const QString& string, const QChar& separator)
{
    clear();
    QRegularExpression regExp(QString(R"(\s?([^\=]+)\=([^%1]+)%1)").arg(separator));

    auto it = regExp.globalMatch(string);
    while(it.hasNext()) {
        auto n = it.next();
        insert(Name(n.captured(1)), Name(n.captured(2)));
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
