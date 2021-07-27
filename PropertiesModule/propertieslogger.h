#ifndef PROPERTIESLOGGER_H
#define PROPERTIESLOGGER_H

#include "property.h"

#include <SharedModule/internal.hpp>

class PropertiesLogger : public Logger
{
    using Super = Logger;
public:
    PropertiesLogger(const QDir& directory);

private:
    NamedUIntProperty m_logSeverity;
    UIntProperty m_logMaxDays;
    BoolProperty m_logConsoleEnabled;
};

#endif // PROPERTIESLOGGER_H
