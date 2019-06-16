#include "logger.h"

#ifdef QT_GUI_LIB
#include <QMessageBox>
#include <QApplication>
#endif

#include <QDir>
#include "SharedModule/External/external.hpp"
#include "SharedModule/Threads/threadsbase.h"
#include "SharedModule/FileSystem/filesguard.h"

Logger::Logger()
    : m_filesGuard(new FilesGuard("log_*.txt", 1))
    , m_severity(Debug)
    , m_printHandler(&Logger::printBoth)
    , m_messageHandler(&Logger::additionalMessageNoOp)
{
    Q_ASSERT(instance() == nullptr);
    instance() = this;

    qInstallMessageHandler(&messageHandler);
}

Logger::~Logger()
{

}

void Logger::SetMaxDays(qint32 maxDays)
{
    instance()->m_filesGuard->SetMaxCount(maxDays);
}

void Logger::SetConsoleEnabled(bool enabled)
{
    if(instance()->m_printHandler == &Logger::printWithoutFile) {
        qCritical() << "Unable to write in file, error captured";
        return;
    }

    if(enabled) {
        instance()->m_printHandler = &Logger::printBoth;
    } else {
        instance()->m_printHandler = &Logger::printWithoutConsole;
    }
}

void Logger::Print(const QString& message)
{
    instance()->print(message);
}

void Logger::SetAdditionalMessageHandler(const QtMessageHandler& messageHandler)
{
    instance()->m_messageHandler = messageHandler;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    Logger* logger = instance();

    QString currentDateTime = QTime::currentTime().toString() + ": ";

    logger->m_messageHandler(type, context, message);

    logger->checkDate();

    switch (type) {
    case QtCriticalMsg:
        ThreadsBase::DoMain([logger, message, currentDateTime]{
            if(logger->m_severity >= Warning) {
                logger->Print("Error " + currentDateTime + message.toLocal8Bit() + "\n");
            }
        });
        break;
    case QtWarningMsg:
        if(logger->m_severity >= Warning) {
            ThreadsBase::DoMain([logger, message, currentDateTime]{
                logger->Print("Warning " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    case QtInfoMsg:
        if(logger->m_severity >= Info) {
            ThreadsBase::DoMain([logger, message, currentDateTime]{
                logger->Print("Info " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    default:
        if(logger->m_severity >= Debug) {
            QString debugLineAndFile = QString("   Loc: [%1:%2] ").arg(context.file, QString::number(context.line));
            ThreadsBase::DoMain([logger, message, currentDateTime, debugLineAndFile]{
                logger->Print(debugLineAndFile + " Info " + currentDateTime + message.toLocal8Bit() + "\n");
            });
        }
        break;
    }
}

Logger*& Logger::instance()
{
    static Logger* result;
    return result;
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
        m_file = new QFile(m_currentDate.toString("'log_'yy'_'MM'_'dd'.txt'"));
        if(m_file->open(QFile::WriteOnly | QFile::Append)) {
            m_fileStream = new QTextStream(m_file.data());

        m_filesGuard->Checkout();
        } else {
            m_printHandler = &Logger::printWithoutFile;
        }
    }
}
