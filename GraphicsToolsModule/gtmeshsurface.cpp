#include "gtmeshsurface.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "SharedGuiModule/decl.h"

GtMeshSurface::GtMeshSurface(qint32 width, qint32 height, qint32 sections)
    : GtMeshIndicesBase(GL_TRIANGLE_STRIP, GL_UNSIGNED_INT)
    , width(width)
    , height(height)
    , sections(sections)

{

}

GtMeshSurface::~GtMeshSurface()
{

}

bool GtMeshSurface::buildMesh()
{
    {
        qint32 sections_plus_one = sections + 1;
        qint32 sections_minus_one = sections - 1;

        vertices_count = pow(sections_plus_one, 2);
        qint32 indexes_without_degenerate_count = sections * (2 * sections + 2);
        indices_count = indexes_without_degenerate_count + (2 * sections - 2);

        TexturedVertex2F* vertices = new TexturedVertex2F[vertices_count];
        qint32* indices = new qint32[indices_count];

        //        QVector<SurfaceVertex> vp(vertices_count);
        //        QVector<qint32> vi(indices_count);
        //        vertices = vp.data();
        //        indices = vi.data();

        float h_step = float(width) / sections;
        float v_step = float(height) / sections;
        float tex_step = 1.f / sections;

        for(qint32 i(0); i < sections_plus_one; i++) {
            for(qint32 j(0); j < sections_plus_one; j++) {
                vertices[i * sections_plus_one + j] = TexturedVertex2F{ Point2F(h_step * j, v_step * i), Point2F(tex_step * j, 1.f - tex_step * i)};
            }
        }

        qint32* indexed_ptr = indices;

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
            *indexed_ptr++ = i + sections * sections_plus_one;
        }
        vbo->bind();
        vbo->allocate(vertices, vertices_count * sizeof(TexturedVertex2F));
        vbo->release();

        vbo_indices->bind();
        vbo_indices->allocate(indices, indices_count * sizeof(qint32));
        vbo_indices->release();

        delete vertices;
        delete indices;
    }

    return true;
}

void GtMeshSurface::bindVAO(OpenGLFunctions* f)
{
    vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(TexturedVertex2F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,2,GL_FLOAT,false,sizeof(TexturedVertex2F),(const void*)sizeof(Point2F));
}
