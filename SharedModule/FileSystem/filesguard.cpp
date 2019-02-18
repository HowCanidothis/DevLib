#include "filesguard.h"

#include "SharedModule/shared_decl.h"

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
