#ifndef GTMESHQUAD2D_H
#define GTMESHQUAD2D_H
#include "gtmeshbase.h"

class GtMeshQuad2D : public GtMesh
{
    GtMeshQuad2D(OpenGLFunctions* f);

public:
    static GtMeshQuad2D* Instance(OpenGLFunctions* f);

    // GtMeshBase interface
protected:
    void updateBuffer();
};

#endif // GTQUADMESH_H
