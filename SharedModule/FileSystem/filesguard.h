#ifndef FILESGUARD_H
#define FILESGUARD_H

#include <QDir>
#include <QDate>
#include <functional>

#include "SharedModule/dispatcher.h"

class FilesGuard
{
public:
    FilesGuard(const QString& pattern, qint32 maxCount, const QDir& dir = QDir::current());

    const QDir& GetDirectory() const { return m_directory; }
    void SetDirectory(const QDir& directory) { m_directory = directory; }
    void SetPattern(const QString& pattern) { m_pattern = pattern; }
    void SetMaxCount(qint32 count) { m_maxCount = count; }
    void Checkout();

    mutable Dispatcher OnFilesRemoved;

private:
    QString m_pattern;
    qint32 m_maxCount;
    QDir m_directory;
};

class FileNamesGeneratorWithGuard
{
    using FOnNewFileName = std::function<void (const QString& newFileName)>;
public:
    FileNamesGeneratorWithGuard(const QString& baseName, const QString& format, qint32 maxFilesCount, const QDir& dir = QDir::current());

    const FilesGuard& GetFilesGuard() const { return m_filesGuard; }

    bool UpdateFileName(const FOnNewFileName& onNewFileName);
    bool UpdateFileNameWithTime(const FOnNewFileName& onNewFileName);

protected:
    QDateTime m_currentDate;
    QString m_baseName;
    QString m_format;
    FilesGuard m_filesGuard;
};

#endif // FILESGUARD_H
