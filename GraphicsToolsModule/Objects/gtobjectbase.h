#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGui/gt_decl.h"
#include "PropertiesModule/internal.hpp"
#include "Shared/external/external.hpp"

class GtObjectBase
{
public:
    virtual void mapProperties(Observer* ) {}
};

class GtObjectActor : public GtObjectBase
{
public:
    virtual void initialize(OpenGLFunctions*)=0;
};

#endif // GTOBJECTBASE_H
