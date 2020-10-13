#include "gtmeshquad2D.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

GtMeshQuad2D::GtMeshQuad2D(OpenGLFunctions* f)
    : GtMesh(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_TexturedVertex2F))
{
    m_buffer->Initialize(f);
    updateBuffer();
}

GtMeshQuad2D* GtMeshQuad2D::Instance(OpenGLFunctions* f)
{
    static GtMeshQuad2D res(f); return &res;
}

void GtMeshQuad2D::updateBuffer()
{
    QVector<TexturedVertex2F> vertices = {
        { Point2F(-1.f, -1.f), Point2F(0.f, 0.f) },
        { Point2F(-1.f, 1.f), Point2F(0.f, 1.f) },
        { Point2F(1.f, -1.f), Point2F(1.f, 0.f) },
        { Point2F(1.f, 1.f), Point2F(1.f, 1.f) },
    };

    m_buffer->UpdateVertexArray(vertices);
}
