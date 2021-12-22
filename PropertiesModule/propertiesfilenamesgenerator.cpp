#include "propertiesfilenamesgenerator.h"


PropertiesFileNamesGenerator::PropertiesFileNamesGenerator(const QString& baseName, const QString& format, qint32 maxFilesCount)
    : Super(baseName, format, maxFilesCount)
    , m_maxCount(Name(QString("FileGuards/%1.%2").arg(baseName, format)), 0, 0, 1000)
{
    m_maxCount = maxFilesCount;

    m_maxCount.Subscribe([this]{
        m_filesGuard.SetMaxCount(m_maxCount);
        m_filesGuard.Checkout();
    });
}
