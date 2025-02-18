#include "streambuffer.h"

QVariant SerializerVersion::CheckVersion(const SerializerVersion& currentVersion, qint64 size) const
{
    if(currentVersion.HashSum != -1 && currentVersion.HashSum != size) {
        return QObject::tr("File is corrupted or could not be loaded.");
    }
    if(currentVersion.Format != Format) {
        return QObject::tr("File Format is not supported - application supports version %1, but file version is %2.").arg(QString::number(Format) , QString::number(currentVersion.Format));
    }
    if(SupportFromVersion != -1 && (quint32)currentVersion.Version < (quint32)SupportFromVersion) {
        return QObject::tr("Version is not supported - application supports versions starting from %1, but file version is %2.").arg(QString::number(SupportFromVersion) , QString::number(currentVersion.Version));
    }
    if((quint32)currentVersion.Version > (quint32)Version) {
        return QObject::tr("Version is not supported - application supports version %1, but file version is %2.").arg(QString::number(Version) , QString::number(currentVersion.Version));
    }
    return QVariant();
}
