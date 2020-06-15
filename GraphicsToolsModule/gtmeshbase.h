#ifndef GTMESHBASE_H
#define GTMESHBASE_H

#include <QOpenGLBuffer>

#include "SharedGuiModule/decl.h"

class QOpenGLVertexArrayObject;

class GtMeshBase
{
public:
    GtMeshBase();
    virtual ~GtMeshBase();

    bool IsVisible() const { return m_visible; }
    void Update();
    void* Map(qint32 offset, qint32 count, QOpenGLBuffer::RangeAccessFlags flags);
    bool UnMap();

    virtual void Initialize(OpenGLFunctions* functions);
    virtual void Draw(gRenderType renderType, OpenGLFunctions* f);

protected:
    ScopedPointer<QOpenGLBuffer> m_vbo;
    ScopedPointer<QOpenGLVertexArrayObject> m_vao;

    qint32 m_verticesCount;

    bool m_visible;

protected:
    virtual bool buildMesh() = 0;
    virtual void bindVAO(OpenGLFunctions*)=0;
};

class GtMeshIndicesBase : public GtMeshBase
{
public:
    GtMeshIndicesBase(gIndicesType itype);
    ~GtMeshIndicesBase();

    virtual void Initialize(OpenGLFunctions* functions) final;
    virtual void Draw(gRenderType renderType, OpenGLFunctions* f) final;

protected:
    ScopedPointer<QOpenGLBuffer> m_vboIndices;

    qint32 m_indicesCount;
    gIndicesType m_indicesType;
};

#endif // GTMESH_H
