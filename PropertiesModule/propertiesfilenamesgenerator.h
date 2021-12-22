#ifndef PROPERTIESFILENAMESGENERATOR_H
#define PROPERTIESFILENAMESGENERATOR_H

#include <SharedModule/internal.hpp>

#include "property.h"

class PropertiesFileNamesGenerator : public FileNamesGeneratorWithGuard
{
    using Super = FileNamesGeneratorWithGuard;
public:
    PropertiesFileNamesGenerator(const QString& baseName, const QString& format, qint32 maxFilesCount);

private:
    IntProperty m_maxCount;
};

#endif // PROPERTIESFILENAMESGENERATOR_H
