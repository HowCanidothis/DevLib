#include "gtmeshquad2D.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

GtMeshBufferQuad2D::GtMeshBufferQuad2D()
    : Super(GtMeshBuffer::VertexType_TexturedVertex2F)
{
}

void GtMeshBufferQuad2D::Initialize(OpenGLFunctions* f, const SP<QOpenGLVertexArrayObject>&)
{
    Super::Initialize(f);
    QVector<TexturedVertex2F> vertices = {
        { Point2F(-1.f, -1.f), Point2F(0.f, 0.f) },
        { Point2F(-1.f, 1.f), Point2F(0.f, 1.f) },
        { Point2F(1.f, -1.f), Point2F(1.f, 0.f) },
        { Point2F(1.f, 1.f), Point2F(1.f, 1.f) },
    };

    UpdateVertexArray(vertices);
}

GtMeshBufferPtr GtMeshBufferQuad2D::Create()
{
    return SharedPointer<GtMeshBufferQuad2D>(new GtMeshBufferQuad2D());
}
