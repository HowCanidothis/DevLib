#ifndef GTMESHQUAD2D_H
#define GTMESHQUAD2D_H
#include "gtmeshbase.h"

class GtMeshBufferQuad2D : public GtMeshBuffer
{
    friend class GtStandardMeshs;
    using Super = GtMeshBuffer;
    GtMeshBufferQuad2D();
    void Initialize(OpenGLFunctions* f, const SP<QOpenGLVertexArrayObject>& vao = nullptr) override;

public:
    static GtMeshBufferPtr Create();
};

#endif // GTQUADMESH_H
