#ifndef GTMESHBASE_H
#define GTMESHBASE_H

#include "SharedGuiModule/decl.h"

class QOpenGLVertexArrayObject;
class QOpenGLBuffer;

class GtMeshBase
{
public:
    GtMeshBase(gRenderType type);
    virtual ~GtMeshBase();

    bool isVisible() const { return visible; }
    void update();
    virtual void initialize(OpenGLFunctions* functions);

    virtual void draw(OpenGLFunctions* f);
protected:
    #pragma pack(1)
    struct TexturedVertex2F
    {
        Point2F position;
        Point2F tex_coord;
    };

    struct ColoredVertex2F
    {
        Point2F position;
        Color3F color;
    };
    #pragma pack()

    ScopedPointer<QOpenGLBuffer> vbo;
    ScopedPointer<QOpenGLVertexArrayObject> vao;

    qint32 vertices_count;
    gRenderType render_type;

    bool visible;
protected:
    virtual bool buildMesh() = 0;
    virtual void bindVAO(OpenGLFunctions*)=0;
};

class GtMeshIndicesBase : public GtMeshBase
{
public:
    GtMeshIndicesBase(gRenderType type, gIndicesType itype);
    ~GtMeshIndicesBase();

    virtual void initialize(OpenGLFunctions* functions) final;

    virtual void draw(OpenGLFunctions* f) final;
protected:
    ScopedPointer<QOpenGLBuffer> vbo_indices;

    qint32 indices_count;
    gIndicesType indices_type;
};

#endif // GTMESH_H
