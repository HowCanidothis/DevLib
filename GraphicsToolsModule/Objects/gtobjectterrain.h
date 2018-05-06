#ifndef GTOBJECTTERRAIN_H
#define GTOBJECTTERRAIN_H

#include "gtobjectbase.h"

class GtObjectTerrain : public GtActor
{
    ScopedPointer<class GtMeshSurface> surface_mesh;
public:
    GtObjectTerrain();

    void initialize(OpenGLFunctions* f) Q_DECL_OVERRIDE;
};

#endif // GTOBJECTTERRAIN_H
