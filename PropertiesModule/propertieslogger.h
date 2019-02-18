#ifndef PROPERTIESLOGGER_H
#define PROPERTIESLOGGER_H

#include "property.h"

class PropertiesLogger
{
public:
    PropertiesLogger();

private:
    NamedUIntProperty m_logSeverity;
    UIntProperty m_logMaxDays;
    BoolProperty m_logConsoleEnabled;
};

#endif // PROPERTIESLOGGER_H
