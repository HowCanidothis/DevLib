#include "filesguard.h"

#include <QDate>

#include <SharedModule/internal.hpp>

FilesGuard::FilesGuard(const QString& pattern, qint32 maxCount)
    : m_pattern(pattern)
    , m_maxCount(maxCount)
    , m_directory(QDir::current())
{
}

void FilesGuard::Checkout()
{
    QStringList entries = m_directory.entryList({ m_pattern }, QDir::Files, QDir::Name);
    qint32 filesToRemove = entries.size() - m_maxCount;
    if(filesToRemove > 0) {
        for(const QString& entry : adapters::range(entries.begin(), entries.begin() + filesToRemove)) {
            QFile::remove(entry);
        }
    }
}

FileNamesGeneratorWithGuard::FileNamesGeneratorWithGuard(const QString& baseName, const QString& format, qint32 maxFilesCount)
    : m_baseName(baseName)
    , m_format(format)
    , m_filesGuard(QString("%1*.%2").arg(baseName, format), maxFilesCount)
{

}

bool FileNamesGeneratorWithGuard::UpdateFileName(const FileNamesGeneratorWithGuard::FOnNewFileName& onNewFileName)
{
    auto currentDate = QDate::currentDate();
    if(m_currentDate != currentDate) {
        m_currentDate = currentDate;
        onNewFileName(currentDate.toString(QString("'%1_'yy'_'MM'_'dd'.%2'").arg(m_baseName, m_format)));
        m_filesGuard.Checkout();
        return true;
    }
    return false;
}
