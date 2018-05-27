#ifndef GTMESHQUAD2D_H
#define GTMESHQUAD2D_H
#include "gtmeshbase.h"

class GtMeshQuad2D : public GtMeshBase
{
    GtMeshQuad2D(OpenGLFunctions* f);
public:

    static GtMeshQuad2D* instance(OpenGLFunctions* f);

    // GtMeshBase interface
protected:
    virtual bool buildMesh() Q_DECL_OVERRIDE;
    void bindVAO(OpenGLFunctions* f) Q_DECL_OVERRIDE;
};

#endif // GTQUADMESH_H
