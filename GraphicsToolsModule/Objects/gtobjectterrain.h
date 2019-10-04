#ifndef GTOBJECTTERRAIN_H
#define GTOBJECTTERRAIN_H

#include "gtobjectbase.h"

class GtObjectTerrain : public GtActor
{
    ScopedPointer<class GtMeshSurface> m_surfaceMesh;
public:
    GtObjectTerrain();

    void Initialize(OpenGLFunctions* f) Q_DECL_OVERRIDE;
};

#endif // GTOBJECTTERRAIN_H
