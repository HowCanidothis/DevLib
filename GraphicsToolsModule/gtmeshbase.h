#ifndef GTMESHBASE_H
#define GTMESHBASE_H

#include <QOpenGLBuffer>

#include "SharedGuiModule/decl.h"

class QOpenGLVertexArrayObject;

class GtMeshBuffer
{
public:
    enum VertexType {
        VertexType_Invalid,
        VertexType_Point3F,
        VertexType_TexturedVertex2F,
        VertexType_StatedVertex3F,
        VertexType_ColoredVertex2F,
        VertexType_ColoredVertex3F,
        VertexType_IntIndex,
    };

    GtMeshBuffer(VertexType vertexType, QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);
    ~GtMeshBuffer();

    void* Map(qint32 offset, qint32 count, QOpenGLBuffer::RangeAccessFlags flags);
    bool UnMap();
    void Initialize(OpenGLFunctions* f);
    void Clear();
    VertexType GetType() const { return m_vertexType; }

    void UpdateVertexArray(const QVector<Point3F>& vertices) { update(vertices, VertexType_Point3F); }
    void UpdateVertexArray(const QVector<ColoredVertex3F>& vertices) { update(vertices, VertexType_ColoredVertex3F); }
    void UpdateVertexArray(const QVector<ColoredVertex2F>& vertices) { update(vertices, VertexType_ColoredVertex2F); }
    void UpdateVertexArray(const QVector<StatedVertex3F>& vertices) { update(vertices, VertexType_StatedVertex3F); }
    void UpdateVertexArray(const QVector<TexturedVertex2F>& vertices) { update(vertices, VertexType_TexturedVertex2F); }
    void UpdateIndicesArray(const QVector<qint32>& indices) { update(indices, VertexType_IntIndex); }

    bool IsValid() const { return m_verticesCount != 0; }
    qint32 GetVerticesCount() const { return m_verticesCount; }

    QOpenGLBuffer* GetVboObject() { return m_vbo.get(); }
    QOpenGLVertexArrayObject* GetVaoObject() { return m_vao.get(); }

protected:
    void setVertexType(VertexType vertexType);

    template<class T>
    void update(const QVector<T>& vertices, VertexType type)
    {
        Q_ASSERT(m_vertexType == type);
        if(m_vbo == nullptr) {
            m_updateOnInitialized = [this, vertices]{
                allocateBuffer(vertices);
            };
        } else {
            allocateBuffer(vertices);
        }
    }
    bool createBuffers();
    template<class T>
    void allocateBuffer(const QVector<T>& vertices)
    {
        m_verticesCount = vertices.size();
        m_vbo->bind();
        m_vbo->allocate(vertices.data(), m_verticesCount * sizeof(T));
        m_vbo->release();
    }

protected:
    ScopedPointer<QOpenGLBuffer> m_vbo;
    ScopedPointer<QOpenGLVertexArrayObject> m_vao;
    std::function<void (OpenGLFunctions*)> m_vaoBinder;
    FAction m_updateOnInitialized;
    qint32 m_verticesCount;
    VertexType m_vertexType;
    QOpenGLBuffer::UsagePattern m_pattern;
};

using GtMeshBufferPtr = SharedPointer<GtMeshBuffer>;

class GtMesh
{
public:
    GtMesh(const GtMeshBufferPtr& buffer);
    virtual ~GtMesh();

    virtual bool IsVisible() const { return m_buffer->IsValid(); }
    const GtMeshBufferPtr& GetBuffer() const { return m_buffer; }

    virtual void Draw(gRenderType renderType, OpenGLFunctions* f);

protected:
    GtMeshBufferPtr m_buffer;
};
using GtMeshPtr = SharedPointer<GtMesh>;

class GtMeshIndices : public GtMesh
{
public:
    GtMeshIndices(const GtMeshBufferPtr& indices, const GtMeshBufferPtr& buffer);
    ~GtMeshIndices();

    bool IsVisible() const final{ return m_buffer->IsValid() && m_indicesBuffer->IsValid(); }
    void Draw(gRenderType renderType, OpenGLFunctions* f) final;

protected:
    GtMeshBufferPtr m_indicesBuffer;
};

#endif // GTMESH_H
