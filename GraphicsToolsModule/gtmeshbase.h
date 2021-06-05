#ifndef GTMESHBASE_H
#define GTMESHBASE_H

#include <QOpenGLBuffer>

#include "SharedGuiModule/decl.h"
#include "GraphicsToolsModule/decl.h"

class QOpenGLVertexArrayObject;

class GtMeshBufferBuilder
{
public:
    void AddComponent(qint32 count, qint32 glType, qint32 typeSize, bool normalized);
    template<class T>
    void AddComponent(qint32 count, bool normalized = false);

    const QVector<std::function<void(OpenGLFunctions*)>>& GetResult() const { return m_result; }
private:
    qint32 m_currentIndex = 0;
    qint32 m_currentStride = 0;
    qint32 m_size = 0;
    QVector<std::function<void(OpenGLFunctions*)>> m_result;
};

template<>
inline void GtMeshBufferBuilder::AddComponent<float>(qint32 count, bool normalized)
{
    AddComponent(count, GL_FLOAT, sizeof(float), normalized);
}

template<>
inline void GtMeshBufferBuilder::AddComponent<qint32>(qint32 count, bool normalized)
{
    AddComponent(count, GL_INT, sizeof(qint32), normalized);
}

class GtMeshBuffer
{
public:
    enum VertexType {
        VertexType_Invalid,
        VertexType_Point3F,
        VertexType_Vertex3f3f,
        VertexType_TexturedVertex3F,
        VertexType_TexturedVertex2F,
        VertexType_StatedVertex3F,
        VertexType_ColoredVertex2F,
        VertexType_ColoredVertex3F,
        VertexType_3f2f2f,
        VertexType_IntIndex,
        VertexType_Custom
    };

    GtMeshBuffer(VertexType vertexType, QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);
    ~GtMeshBuffer();

    void* Map(qint32 offset, qint32 count, QOpenGLBuffer::RangeAccessFlags flags);
    bool UnMap();
    virtual void Initialize(OpenGLFunctions* f);
    void UpdateVao(OpenGLFunctions* f) { m_vaoBinder(f); }
    void Clear();
    VertexType GetType() const { return m_vertexType; }

    void UpdateVertexArray(const QVector<Vertex3f3f>& vertices) { update(vertices, VertexType_Vertex3f3f); }
    void UpdateVertexArray(const QVector<Point3F>& vertices) { update(vertices, VertexType_Point3F); }
    void UpdateVertexArray(const QVector<ColoredVertex3F>& vertices) { update(vertices, VertexType_ColoredVertex3F); }
    void UpdateVertexArray(const QVector<ColoredVertex2F>& vertices) { update(vertices, VertexType_ColoredVertex2F); }
    void UpdateVertexArray(const QVector<StatedVertex3F>& vertices) { update(vertices, VertexType_StatedVertex3F); }
    void UpdateVertexArray(const QVector<TexturedVertex2F>& vertices) { update(vertices, VertexType_TexturedVertex2F); }
    void UpdateVertexArray(const QVector<TexturedVertex3F>& vertices) { update(vertices, VertexType_TexturedVertex3F); }
    void UpdateVertexArray(const QVector<Vertex3f2f2f>& vertices) { update(vertices, VertexType_3f2f2f); }
    void UpdateIndicesArray(const QVector<qint32>& indices) { update(indices, VertexType_IntIndex); }
    template<class T>
    void UpdateVertexArray(const QVector<T>& vertices, const GtMeshBufferBuilder& meshBuilder) { m_builder = meshBuilder; update(vertices, VertexType_Custom); }

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
    GtMeshBufferBuilder m_builder;
};

class GtMesh
{
public:
    GtMesh(const GtMeshBufferPtr& buffer);
    virtual ~GtMesh();

    void SetVisible(bool visible) { m_visible = visible; }
    virtual bool IsVisible() const { return m_visible && m_buffer->IsValid(); }
    const GtMeshBufferPtr& GetBuffer() const { return m_buffer; }

    virtual void Draw(gRenderType renderType, OpenGLFunctions* f);

protected:
    GtMeshBufferPtr m_buffer;
    bool m_visible;
};

class GtMeshIndices : public GtMesh
{
public:
    GtMeshIndices(const GtMeshBufferPtr& indices, const GtMeshBufferPtr& buffer);
    ~GtMeshIndices();

    bool IsVisible() const final{ return m_visible && m_buffer->IsValid() && m_indicesBuffer->IsValid(); }
    void Draw(gRenderType renderType, OpenGLFunctions* f) final;

protected:
    GtMeshBufferPtr m_indicesBuffer;
};

class GtStandardMeshs
{
    friend class GtRenderer;
    GtStandardMeshs();
public:
    const GtMeshPtr& GetQuadMesh2D() const { return m_quad2DMesh; }

private:
    void initialize(OpenGLFunctions* f);
    void reset(OpenGLFunctions* f);

private:
    GtMeshPtr m_quad2DMesh;
};


#endif // GTMESH_H
