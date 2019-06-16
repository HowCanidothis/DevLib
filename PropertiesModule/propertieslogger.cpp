#include "propertieslogger.h"

#include <QObject>

#include <SharedModule/Logger/logger.h>

PropertiesLogger::PropertiesLogger()
    : m_logSeverity("Log/Severity", Logger::Debug)
    , m_logMaxDays("Log/MaxDays", 1, 1, 30)
    , m_logConsoleEnabled("Log/ConsoleEnabled", true)
{
    m_logSeverity.SetNames({ QObject::tr("Error"), QObject::tr("Warning"), QObject::tr("Info"), QObject::tr("Debug") });

    m_logSeverity.Subscribe([this]{
        Logger::SetSeverity((Logger::ESeverity)(int)m_logSeverity);
    });

    m_logMaxDays.Subscribe([this]{
        Logger::SetMaxDays(m_logMaxDays);
    });

    m_logConsoleEnabled.Subscribe([this]{
        Logger::SetConsoleEnabled(m_logConsoleEnabled);
    });
}
