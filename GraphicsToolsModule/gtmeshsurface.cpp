#include "gtmeshsurface.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "SharedGuiModule/decl.h"

GtMeshSurface::GtMeshSurface(qint32 width, qint32 height, qint32 sections)
    : GtMeshIndices(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_IntIndex), ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_TexturedVertex2F))
    , m_width(width)
    , m_height(height)
    , m_sections(sections)
{
    updateBuffer();
}

GtMeshSurface::~GtMeshSurface()
{

}

void GtMeshSurface::updateBuffer()
{
    /*    <--w-->
     *   /_______\
     *  /|_s_|___|
     * h |___|___|
     *  \|___|___|
     *  s - count of sections at the line. The same count at the column, total sections count is s * s
     *  w - real width
     *  h - real height
    */
    qint32 sections_plus_one = m_sections + 1;
    qint32 sections_minus_one = m_sections - 1;

    auto verticesCount = pow(sections_plus_one, 2);
    qint32 indexes_without_degenerate_count = m_sections * (2 * m_sections + 2);
    auto indicesCount = indexes_without_degenerate_count + (2 * m_sections - 2);

    QVector<TexturedVertex2F> vertices(verticesCount);
    QVector<qint32> indices(indicesCount);

    //        QVector<SurfaceVertex> vp(vertices_count);
    //        QVector<qint32> vi(indices_count);
    //        vertices = vp.data();
    //        indices = vi.data();

    float h_step = float(m_width) / m_sections;
    float v_step = float(m_height) / m_sections;
    float tex_step = 1.f / m_sections;

    for(qint32 i(0); i < sections_plus_one; i++) {
        for(qint32 j(0); j < sections_plus_one; j++) {
            vertices[i * sections_plus_one + j] = TexturedVertex2F{ Point2F(h_step * j, v_step * i), Point2F(tex_step * j, 1.f - tex_step * i)};
        }
    }

    auto indexed_ptr = indices.begin();

    for(qint32 j(0); j < sections_minus_one; j++) {
        for(qint32 i(0); i < sections_plus_one; i++) {
            *indexed_ptr++ = i + j * sections_plus_one;
            *indexed_ptr++ = i + (j + 1) * sections_plus_one;
        }
        *indexed_ptr++ = *(indexed_ptr - 1);
        *indexed_ptr++ = (j + 1) * sections_plus_one;
    }

    qint32 offset = sections_minus_one * sections_plus_one;
    for(qint32 i(0); i < sections_plus_one; i++) {
        *indexed_ptr++ = i + offset;
        *indexed_ptr++ = i + m_sections * sections_plus_one;
    }

    m_buffer->UpdateVertexArray(vertices);
    m_indicesBuffer->UpdateIndicesArray(indices);
}
