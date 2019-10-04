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

PropertiesFileNamesGeneratorFileStream::PropertiesFileNamesGeneratorFileStream(int64_t key, int32_t version, const QString& baseName, const QString& format, qint32 maxFilesCount)
    : m_fileNamesGenerator(baseName, format, maxFilesCount)
    , m_key(key)
    , m_version(version)
{}

void PropertiesFileNamesGeneratorFileStream::updateStream()
{
    m_fileNamesGenerator.UpdateFileName([this](const QString& newFileName){
        m_file = new QFile(newFileName);
        if(m_file->open(QFile::WriteOnly)) {
            m_stream = new QStreamBufferWrite(m_key, m_version, m_file.get());
        } else {
            qCritical() << newFileName << "cannot be created. Stream was not initialized";
        }
    });
}
