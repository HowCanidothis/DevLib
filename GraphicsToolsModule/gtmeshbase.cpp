#include "gtmeshbase.h"

#include <QOpenGLVertexArrayObject>

#include "gtmeshquad2D.h"

GtMeshBufferBuilder& GtMeshBufferBuilder::AddComponent(qint32 count, qint32 glType, qint32 typeSize, bool normalized)
{
    qint32 attributeIndex = m_currentIndex;
    qint32 currentStride = m_currentStride;
    m_result.append([this, attributeIndex, count, normalized, currentStride, glType](OpenGLFunctions* f){
        f->glEnableVertexAttribArray(attributeIndex);
        f->glVertexAttribPointer(attributeIndex, count, glType, normalized, m_currentStride, (void*)currentStride);
    });
    auto size = typeSize * count;
    m_currentStride += size;
    m_size += size;
    m_currentIndex++;
    return *this;
}

GtMeshBuffer::GtMeshBuffer(GtMeshBuffer::VertexType vertexType, QOpenGLBuffer::UsagePattern pattern)
    : m_verticesCount(0)
    , m_vertexType(VertexType_Invalid)
    , m_pattern(pattern)
{
    setVertexType(vertexType);
}

GtMeshBuffer::~GtMeshBuffer()
{

}

void GtMeshBuffer::Initialize(OpenGLFunctions* f)
{
    Q_ASSERT(m_vbo == nullptr && m_vao == nullptr);
    m_verticesCount = 0;

    if(createBuffers()) {
        if(m_updateOnInitialized != nullptr) {
            m_updateOnInitialized();
            m_updateOnInitialized = nullptr;
        }
        m_vaoBinder(f);
    }
}

void GtMeshBuffer::Clear()
{
    m_verticesCount = 0;
    m_vbo->bind();
    m_vbo->allocate(0);
    m_vbo->release();
}

void GtMeshBuffer::setVertexType(GtMeshBuffer::VertexType vertexType)
{
    switch(vertexType){
    case VertexType_TexturedVertex2F:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(TexturedVertex2F),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribPointer(1,2,GL_FLOAT,false,sizeof(TexturedVertex2F),(const void*)sizeof(Point2F));
        }; break;
    case VertexType_Vertex3f3f:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(Vertex3f3f),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(Vertex3f3f),(const void*)sizeof(Point3F));
        }; break;
    case VertexType_3f2f2f:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(Vertex3f2f2f),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribPointer(1,2,GL_FLOAT,false,sizeof(Vertex3f2f2f),(const void*)sizeof(Point3F));
            f->glEnableVertexAttribArray(2);
            f->glVertexAttribPointer(2,2,GL_FLOAT,false,sizeof(Vertex3f2f2f),(const void*)(sizeof(Point3F) + sizeof(Point2F)));
        }; break;
    case VertexType_TexturedVertex3F:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(TexturedVertex3F),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribPointer(1,2,GL_FLOAT,false,sizeof(TexturedVertex3F),(const void*)sizeof(Point3F));
        }; break;
    case VertexType_StatedVertex3F:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(StatedVertex3F),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribIPointer(1,1,GL_UNSIGNED_BYTE,sizeof(StatedVertex3F),(const void*)sizeof(Point3F));
            f->glEnableVertexAttribArray(2);
            f->glVertexAttribIPointer(2,1,GL_UNSIGNED_BYTE,sizeof(StatedVertex3F),(const void*)(sizeof(Point3F) + sizeof(quint8)));
        }; break;
    case VertexType_ColoredVertex2F:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(ColoredVertex2F),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex2F),(const void*)sizeof(Point2F));
        }; break;
    case VertexType_ColoredVertex3F:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(ColoredVertex3F),nullptr);
            f->glEnableVertexAttribArray(1);
            f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex3F),(const void*)sizeof(Point3F));
        }; break;
    case VertexType_Point3F:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(Point3F),nullptr);
        }; break;
    case VertexType_IntIndex:
        m_vaoBinder = [](OpenGLFunctions*){};
        break;
    case VertexType_Custom:
        m_vaoBinder = [this](OpenGLFunctions* f) {
            QOpenGLVertexArrayObject::Binder binder(m_vao.data());
            m_vbo->bind();
            for(const auto& functor : m_builder.GetResult()) {
                functor(f);
            }
        }; break;
    default: Q_ASSERT(false);
    }

    m_vertexType = vertexType;
}

void* GtMeshBuffer::Map(qint32 offset, qint32 count, QOpenGLBuffer::RangeAccessFlags flags)
{
    m_vbo->bind();
    return m_vbo->mapRange(offset, count, flags);
}

bool GtMeshBuffer::UnMap()
{
    auto result = m_vbo->unmap();
    m_vbo->release();
    return result;
}

bool GtMeshBuffer::createBuffers()
{
    if(m_vbo == nullptr) {
        if(m_vertexType == VertexType_IntIndex) {
            m_vbo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        } else {
            m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        }
        m_vbo->create();
        m_vbo->setUsagePattern(m_pattern);
    }

    if(m_vertexType == VertexType_IntIndex) {
        return m_vbo->isCreated();
    }

    if(m_vao == nullptr) {
        m_vao = new QOpenGLVertexArrayObject();
        m_vao->create();
    }
    return m_vao->isCreated() && m_vbo->isCreated();
}

GtMesh::GtMesh(const GtMeshBufferPtr& buffer)
    : m_buffer(buffer)
    , m_visible(true)
{

}

GtMesh::~GtMesh()
{
}

void GtMesh::Draw(gRenderType renderType, OpenGLFunctions* f)
{
    QOpenGLVertexArrayObject::Binder binder(m_buffer->GetVaoObject());
    f->glDrawArrays(renderType, 0, m_buffer->GetVerticesCount());
}

GtMeshIndices::GtMeshIndices(const GtMeshBufferPtr& indices, const GtMeshBufferPtr& buffer)
    : GtMesh(buffer)
    , m_indicesBuffer(indices)
{
    Q_ASSERT(m_indicesBuffer->GetType() == GtMeshBuffer::VertexType_IntIndex);
}

GtMeshIndices::~GtMeshIndices()
{

}

void GtMeshIndices::Draw(gRenderType renderType, OpenGLFunctions* f)
{
    QOpenGLVertexArrayObject::Binder binder(m_buffer->GetVaoObject());
    m_indicesBuffer->GetVboObject()->bind();
    f->glDrawElements(renderType, m_indicesBuffer->GetVerticesCount(), GL_UNSIGNED_INT, (const void*)0);
}

GtStandardMeshs::GtStandardMeshs()
    : m_quad2DMesh(::make_shared<GtMesh>(GtMeshBufferQuad2D::Create()))
{
}

void GtStandardMeshs::initialize(OpenGLFunctions* f)
{
    m_quad2DMesh->GetBuffer()->Initialize(f);
}
