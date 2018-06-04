#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include "SharedModule/external/external.hpp"
#include "SharedModule/array.h"

class GtObjectBase
{
public:
    virtual void mapProperties(Observer* ) {}
};

class GtPrimitiveActor : public GtObjectBase
{
    BoundingBox bounding_box;

public:
    void addComponent(GtPrimitiveActor* actor) { components.Append(actor); }
    const BoundingBox& getBoundingBox() const { return bounding_box; }
public:
    virtual void initialize(OpenGLFunctions*)=0;

protected:
    friend class GtActor;
    bool updateBoundingBox() { return updateBoundingBox(bounding_box); }

    virtual bool updateBoundingBox(BoundingBox&) { return false; }
    virtual void UpdateTransform() {}

protected:
    ArrayPointers<GtPrimitiveActor> components;
};

class GtActor : public GtPrimitiveActor
{
public:
    virtual bool updateBoundingBox(BoundingBox&) Q_DECL_OVERRIDE;
};

#endif // GTOBJECTBASE_H
