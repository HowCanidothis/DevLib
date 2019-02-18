#ifndef FILESGUARD_H
#define FILESGUARD_H

#include <QDir>

class FilesGuard
{
public:
    FilesGuard(const QString& pattern, qint32 maxCount);

    void SetDirectory(const QDir& directory) { m_directory = directory; }
    void SetPattern(const QString& pattern) { m_pattern = pattern; }
    void SetMaxCount(qint32 count) { m_maxCount = count; }
    void Checkout();

private:
    QString m_pattern;
    qint32 m_maxCount;
    QDir m_directory;
};

#endif // FILESGUARD_H
