#ifndef LOGGER_H
#define LOGGER_H

#include <QDate>

#include "SharedModule/smartpointersadapters.h"

class Logger : public Singletone<Logger>
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

    Logger(const QDir& directory);
    ~Logger();

    static void EnableLogging(bool enabled);
    static void SetSeverity(ESeverity severity) { GetInstance().m_severity = severity; }
    static void SetMaxDays(qint32 maxDays);
    static void SetConsoleEnabled(bool enabled);

    static void Print(const QString& message);
    static void SetAdditionalMessageHandler(const QtMessageHandler& messageHandler);
    static void Terminate();

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& message);

    void print(const QString& message);

    void printNo(const QString& message);
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
    FPrintToConsole m_printHandlerBefore;
    QtMessageHandler m_messageHandler;
};

#endif // LOGGER_H
