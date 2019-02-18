#ifndef LOGGER_H
#define LOGGER_H

#include <QDate>

#include "SharedModule/smartpointersadapters.h"

class Logger
{
    typedef void (Logger::*FPrintToConsole)(const QString& message);
public:
    enum ESeverity
    {
        Error,
        Warning,
        Info,
        Debug,
    };

    Logger();
    ~Logger();

    static void SetSeverity(ESeverity severity) { instance()->m_severity = severity; }
    static void SetMaxDays(qint32 maxDays);
    static void SetConsoleEnabled(bool enabled);

    static void Print(const QString& message);
    static void SetAdditionalMessageHandler(const QtMessageHandler& messageHandler);

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& message);
    static Logger*& instance();

    void print(const QString& message);

    void printWithoutFile(const QString& message);
    void printBoth(const QString& message);
    void printWithoutConsole(const QString& message);
    void checkDate();

    static void additionalMessageNoOp(QtMsgType, const QMessageLogContext&, const QString&) {}

private:
    ScopedPointer<class FilesGuard> m_filesGuard;
    ScopedPointer<class QFile> m_file;
    ScopedPointer<class QTextStream> m_fileStream;
    ESeverity m_severity;
    QDate m_currentDate;
    qint32 m_currentDay;
    FPrintToConsole m_printHandler;
    QtMessageHandler m_messageHandler;
};

#endif // LOGGER_H
