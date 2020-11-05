#ifndef GTMESHLOADER_H
#define GTMESHLOADER_H

#include <SharedModule/internal.hpp>

#include "gtmeshbase.h"

class GtMeshLoader
{
public:
    static GtMeshPtr LoadObj(const QString& fileName);
};
#endif // GTMESHLOADER_H
