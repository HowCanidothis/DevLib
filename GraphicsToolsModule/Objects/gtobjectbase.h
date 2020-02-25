#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include <SharedModule/External/external.hpp>
#include <SharedModule/internal.hpp>

class GtObjectBase
{
public:
    virtual void MapProperties(Observer* ) {}
};

class GtDrawableBase : public GtObjectBase
{
public:
    virtual ~GtDrawableBase() {}

    virtual void Draw(OpenGLFunctions* f) = 0;
};

class GtInteractableBase : public GtDrawableBase
{
public:
    virtual ~GtInteractableBase() {}

    virtual bool ContainsPoint(const Point3F& point) const = 0;
    virtual float DistanceToPoint(const Point3F& point) const = 0;
};


#endif // GTOBJECTBASE_H
