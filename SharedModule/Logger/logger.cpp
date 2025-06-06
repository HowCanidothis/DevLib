#include "logger.h"

#include <QDir>
#include "SharedModule/External/external.hpp"
#include "SharedModule/Threads/threadsbase.h"
#include "SharedModule/FileSystem/filesguard.h"

Logger::Logger(const QDir& directory)
    : m_filesGuard(new FilesGuard("log_*.txt", 1, directory))
    , m_severity(Debug)
    , m_printHandler(&Logger::printBoth)
    , m_messageHandler(&Logger::additionalMessageNoOp)
{
    qInstallMessageHandler(&messageHandler);
}

Logger::~Logger()
{

}

void Logger::EnableLogging(bool enabled)
{
    auto& logger = GetInstance();
    if(!enabled && logger.m_printHandler != &Logger::printNo) {
        logger.m_printHandlerBefore = logger.m_printHandler;
        logger.m_printHandler = &Logger::printNo;
    } else if(enabled && logger.m_printHandler == &Logger::printNo){
        logger.m_printHandler = logger.m_printHandlerBefore;
    }
}

void Logger::SetMaxDays(qint32 maxDays)
{
    GetInstance().m_filesGuard->SetMaxCount(maxDays);
}

void Logger::SetConsoleEnabled(bool enabled)
{
    auto& logger = GetInstance();
    if(logger.m_printHandler == &Logger::printWithoutFile) {
        qCritical() << "Unable to write in file, error captured";
        return;
    }

    if(enabled) {
        logger.m_printHandler = &Logger::printBoth;
    } else {
        logger.m_printHandler = &Logger::printWithoutConsole;
    }
}

void Logger::Terminate()
{
    Logger::EnableLogging(false);
    GetInstance().m_fileStream = nullptr;
    GetInstance().m_file = nullptr;
}

void Logger::printNo(const QString&)
{

}

void Logger::Print(const QString& message)
{
    GetInstance().print(message);
}

void Logger::SetAdditionalMessageHandler(const QtMessageHandler& messageHandler)
{
    GetInstance().m_messageHandler = messageHandler;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    Logger& logger = GetInstance();

    QString currentDateTime = QTime::currentTime().toString() + ": ";

    logger.m_messageHandler(type, context, message);

    logger.checkDate();

    switch (type) {
    case QtCriticalMsg:
        if(logger.m_severity >= Error) {
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[message, currentDateTime]{
                GetInstance().Print("Error " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    case QtWarningMsg:
        if(logger.m_severity >= Warning) {
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[message, currentDateTime]{
                GetInstance().Print("Warning " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    case QtInfoMsg:
        if(logger.m_severity >= Info) {
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[message, currentDateTime]{
                GetInstance().Print("Info " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    default:
        if(logger.m_severity >= Debug) {
            QString debugLineAndFile = QString("   Loc: [%1:%2] ").arg(context.file, QString::number(context.line));
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[message, currentDateTime, debugLineAndFile]{
                GetInstance().Print(debugLineAndFile + " Info " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    }
}

void Logger::print(const QString& message)
{
    (this->*m_printHandler)(message);
}

void Logger::printWithoutFile(const QString& message)
{
    fprintf(stderr, message.toLocal8Bit());
}

void Logger::printBoth(const QString& message)
{
    fprintf(stderr, message.toLocal8Bit());
    *m_fileStream << message;
    m_fileStream->flush();
}

void Logger::printWithoutConsole(const QString& message)
{
    *m_fileStream << message;
    m_fileStream->flush();
}

void Logger::checkDate()
{
    QDate currentDate = QDate::currentDate();
    if(m_currentDate != currentDate) {
        m_currentDate = currentDate;
        m_file = new QFile(m_filesGuard->GetDirectory().filePath(m_currentDate.toString("'log_'yy'_'MM'_'dd'.txt'")));
        if(m_file->open(QFile::WriteOnly | QFile::Append)) {
            m_fileStream = new QTextStream(m_file.data());
            m_filesGuard->Checkout();
        } else {
            m_printHandler = &Logger::printWithoutFile;
        }
    }
}
