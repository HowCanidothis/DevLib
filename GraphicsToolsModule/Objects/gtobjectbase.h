#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include "SharedModule/external/external.hpp"
#include "SharedModule/array.h"

class GtObjectBase
{
public:
    virtual void MapProperties(Observer* ) {}
};

class GtPrimitiveActor : public GtObjectBase
{
    BoundingBox m_boundingBox;

public:
    void AddComponent(GtPrimitiveActor* actor) { m_components.Append(actor); }
    const BoundingBox& GetBoundingBox() const { return m_boundingBox; }

public:
    virtual void Initialize(OpenGLFunctions*)=0;

protected:
    friend class GtActor;
    bool updateBoundingBox() { return updateBoundingBox(m_boundingBox); }

    virtual bool updateBoundingBox(BoundingBox&) { return false; }
    virtual void updateTransform() {}

protected:
    ArrayPointers<GtPrimitiveActor> m_components;
};

class GtActor : public GtPrimitiveActor
{
protected:
    virtual bool updateBoundingBox(BoundingBox&) Q_DECL_OVERRIDE;
};

#endif // GTOBJECTBASE_H
