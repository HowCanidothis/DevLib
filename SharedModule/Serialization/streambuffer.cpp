#include "streambuffer.h"

QVariant SerializerVersion::CheckVersion(const SerializerVersion& currentVersion, bool strictVersion, qint64 size) const
{
    if(currentVersion.HashSum != -1 && currentVersion.HashSum != size) {
        return QObject::tr("File corrupted or cannot be loaded");
    }
    if(currentVersion.Format != Format) {
        return QObject::tr("Format error - expected %1, but file version is %2").arg(QString::number(Format) , QString::number(currentVersion.Format));
    }
    if(strictVersion) {
        if((quint32)currentVersion.Version != (quint32)Version) {
            return QObject::tr("Version is not supported - application supported version is %1, but file version is %2").arg(QString::number(Version) , QString::number(currentVersion.Version));
        }
    } else if((quint32)currentVersion.Version > (quint32)Version) {
        return QObject::tr("Future version error - application supported version is %1, but file version is %2").arg(QString::number(Version) , QString::number(currentVersion.Version));
    }
    return QVariant();
}