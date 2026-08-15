#include "gtmeshbase.h"

#include <QOpenGLVertexArrayObject>

#include "gtmeshquad2D.h"

// ============================================================================
// GtMeshBufferBuilder Implementation
// ============================================================================

GtMeshBufferBuilder& GtMeshBufferBuilder::AddComponent(qint32 count, qint32 glType, qint32 typeSize, bool normalized, qint32 divisor)
{
    qint32 attributeIndex = m_currentIndex;
    qint32 currentStride = m_currentStride;
    m_result.append([this, attributeIndex, count, normalized, currentStride, glType, divisor](OpenGLFunctions* f){
        f->glEnableVertexAttribArray(attributeIndex);
        f->glVertexAttribPointer(attributeIndex, count, glType, normalized, m_currentStride, (void*)currentStride);
        f->glVertexAttribDivisor(attributeIndex, divisor);
    });
    auto size = typeSize * count;
    m_currentStride += size;
    m_size += size;
    m_currentIndex++;
    return *this;
}

// ============================================================================
// GtMeshBuffer Implementation
// ============================================================================

GtMeshBuffer::GtMeshBuffer(VertexType vertexType, QOpenGLBuffer::UsagePattern pattern)
    : m_verticesCount(0)
    , m_vertexType(VertexType_Invalid)
    , m_pattern(pattern)
{
    m_vertexType = vertexType;
}

GtMeshBuffer::~GtMeshBuffer()
{
    // The VBO can be cleanly destroyed here since it belongs to a shared memory group
    if (m_vbo) {
        m_vbo->destroy();
    }
}

void GtMeshBuffer::SetShared()
{
    m_shared = new std::pair<QMutex, std::atomic_bool>();
    m_shared->second = false;
}

void GtMeshBuffer::Initialize(OpenGLFunctions*)
{
    if(m_shared != nullptr) {
        if(m_shared->second.load(std::memory_order_acquire)) {
            return;
        }
        QMutexLocker locker(&m_shared->first);
        if(m_shared->second.load(std::memory_order_relaxed)) {
            return;
        }
        if (createBuffers()) {
            if (m_updateOnInitialized != nullptr) {
                m_updateOnInitialized();
                m_updateOnInitialized = nullptr;
            }
            m_shared->second.store(true, std::memory_order_release);
        }
    } else {
        if(m_vbo != nullptr) {
            return;
        }

        if (createBuffers()) {
            if (m_updateOnInitialized != nullptr) {
                m_updateOnInitialized();
                m_updateOnInitialized = nullptr;
            }
        }
    }
}

void GtMeshBuffer::Clear()
{
    m_verticesCount = 0;
    if (m_vbo && m_vbo->isCreated()) {
        m_vbo->bind();
        m_vbo->allocate(nullptr, 0);
        m_vbo->release();
    }
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
    if (m_vbo == nullptr) {
        if (m_vertexType == VertexType_IntIndex) {
            m_vbo = make_scoped<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
        } else {
            m_vbo = make_scoped<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
        }
        m_vbo->create();
        m_vbo->setUsagePattern(m_pattern);
    }
    return m_vbo->isCreated();
}

// Emits the specific layout attributes down to a target context-private VAO
void GtMeshBuffer::ExecuteLayoutBinding(OpenGLFunctions* f, QOpenGLVertexArrayObject* targetVao)
{
    if (m_vertexType == VertexType_IntIndex || !m_vbo || !m_vbo->isCreated()) {
        return;
    }

    m_vbo->bind();

    switch (m_vertexType) {
    case VertexType_Vertex3f3f3f:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f3f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f3f3f, Position)));

        // Attribute 1: Normal (Location 1 in shader)
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f3f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f3f3f, Normal)));

        // Attribute 2: TexCoord (Location 2 in shader)
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f3f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f3f3f, TexCoord)));
        break;

    case VertexType_TexturedVertex2F:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex2F),
                                 reinterpret_cast<const void*>(offsetof(TexturedVertex2F, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex2F),
                                 reinterpret_cast<const void*>(offsetof(TexturedVertex2F, TexCoord)));
        break;

    case VertexType_Vertex3f3f:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f3f, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f3f, Normal)));
        break;

    case VertexType_3f2f2f:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f2f2f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f2f2f, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f2f2f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f2f2f, A))); // update names if needed
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f2f2f),
                                 reinterpret_cast<const void*>(offsetof(Vertex3f2f2f, B)));
        break;

    case VertexType_TexturedVertex3F:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3F),
                                 reinterpret_cast<const void*>(offsetof(TexturedVertex3F, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3F),
                                 reinterpret_cast<const void*>(offsetof(TexturedVertex3F, TexCoord)));
        break;

    case VertexType_StatedVertex3F:
        // Core Fix: offsetof safely bridges the byte alignment padding between point and state variables
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StatedVertex3F),
                                 reinterpret_cast<const void*>(offsetof(StatedVertex3F, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(StatedVertex3F),
                                  reinterpret_cast<const void*>(offsetof(StatedVertex3F, State)));
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(StatedVertex3F),
                                  reinterpret_cast<const void*>(offsetof(StatedVertex3F, Transparency)));
        break;

    case VertexType_ColoredVertex2F:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex2F),
                                 reinterpret_cast<const void*>(offsetof(ColoredVertex2F, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex2F),
                                 reinterpret_cast<const void*>(offsetof(ColoredVertex2F, Color)));
        break;

    case VertexType_ColoredVertex3F:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex3F),
                                 reinterpret_cast<const void*>(offsetof(ColoredVertex3F, Position)));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex3F),
                                 reinterpret_cast<const void*>(offsetof(ColoredVertex3F, Color)));
        break;

    case VertexType_Point3F:
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point3F), nullptr);
        break;

    case VertexType_Custom:
        for (const auto& functor : m_builder.GetResult()) {
            functor(f);
        }
        break;

    default:
        Q_ASSERT(false);
    }
}

// ============================================================================
// GtMesh Base Implementation
// ============================================================================

GtMesh::GtMesh(const GtMeshBufferPtr& buffer)
    : m_buffer(buffer)
    , m_visible(true)
{
}

GtMesh::~GtMesh()
{
    // Local safe deletion because GtMesh objects do not jump threads
    if (m_vao && m_vao->isCreated()) {
        m_vao->destroy();
    }
}

void GtMesh::BindLayoutForCurrentContext(OpenGLFunctions* f, QOpenGLBuffer* indices, GtMeshBuffer* instanceBuffer)
{
    // Lazy Evaluation: Create VAO locally on whichever thread owns this object instance
    if (m_vao == nullptr) {
        m_vao = make_scoped<QOpenGLVertexArrayObject>();
        m_vao->create();

        m_vao->bind();

        // Pass layout mapping configuration instructions to target local VAO
        m_buffer->ExecuteLayoutBinding(f, m_vao.get());

        // Lock the Index EBO state inside this VAO context permanently if required
        if (indices != nullptr) {
            indices->bind();
        }

        if (instanceBuffer != nullptr) {
            instanceBuffer->ExecuteLayoutBinding(f, m_vao.get());
        }
    }

    m_vao->bind();
}

void GtMesh::Draw(gRenderType renderType, OpenGLFunctions* f)
{
    if (!IsVisible()) return;

    BindLayoutForCurrentContext(f);
    f->glDrawArrays(renderType, 0, m_buffer->GetVerticesCount());
    m_vao->release();
}

void GtMesh::DrawInstanced(gRenderType renderType, OpenGLFunctions* f, qint32 instancesCount)
{
    if (!IsVisible()) return;

    BindLayoutForCurrentContext(f, nullptr, m_instanceBuffer.get());
    f->glDrawArraysInstanced(renderType, 0, m_buffer->GetVerticesCount(), instancesCount);
    m_vao->release();
}

// ============================================================================
// GtMeshIndices Implementation
// ============================================================================

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
    if (!IsVisible()) return;

    // Passes down EBO handler to cache it into our context-bound VAO permanently
    BindLayoutForCurrentContext(f, m_indicesBuffer->GetVboObject());

    f->glDrawElements(renderType, m_indicesBuffer->GetVerticesCount(), GL_UNSIGNED_INT, nullptr);
    m_vao->release();
}

void GtMeshIndices::DrawInstanced(gRenderType renderType, OpenGLFunctions* f, qint32 instancesCount)
{
    if (!IsVisible()) return;

    BindLayoutForCurrentContext(f, m_indicesBuffer->GetVboObject(), m_instanceBuffer.get());
    f->glDrawElementsInstanced(renderType, m_indicesBuffer->GetVerticesCount(), GL_UNSIGNED_INT, nullptr, instancesCount);
    m_vao->release();
}

// ============================================================================
// GtStandardMeshs Implementation
// ============================================================================

GtStandardMeshs::GtStandardMeshs()
    : m_quad2DMesh(new GtMesh(GtMeshBufferQuad2D::Create()))
{
}

void GtStandardMeshs::initialize(OpenGLFunctions* f)
{
    m_quad2DMesh->GetBuffer()->Initialize(f);
}

void GtStandardMeshs::reset(OpenGLFunctions* f)
{
    Q_UNUSED(f);
    m_quad2DMesh->GetBuffer()->Clear();
}
