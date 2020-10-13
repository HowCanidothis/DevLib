#include "gtmeshcircle2D.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <qmath.h>

Circle2D::Circle2D(const Point2F& pos, const Point2F& radius, const Color3F& color)
    : Position(pos)
    , Radius(radius)
    , Color(color)
{

}

GtMeshCircle2D::GtMeshCircle2D()
    : GtMesh(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_ColoredVertex2F, QOpenGLBuffer::DynamicDraw))
{

}

void GtMeshCircle2D::Resize(qint32 size)
{
    m_circles.resizeAndAllocate(size);
}

void GtMeshCircle2D::Clear()
{
    m_circles.Clear();
}

Circle2D* GtMeshCircle2D::Add(float x, float y, float r, const Color3F& color)
{
    Circle2D* result = new Circle2D(Point2F(x,y),Point2F(r,r), color);
    m_circles.InsertSortedUnique(result);
    return result;
}

void GtMeshCircle2D::Remove(Circle2D* circle)
{
    delete circle;
    auto find = m_circles.FindSorted(circle);
    m_circles.Remove(find);
}

void GtMeshCircle2D::UpdateBuffer()
{
    if(!m_circles.Size()) {
        m_buffer->Clear();
        return;
    }
    qint32 vertices_per_circle = 12 + 1 + 1;
    auto verticesCount = vertices_per_circle * m_circles.Size();
    QVector<ColoredVertex2F> vertices(verticesCount);
    ColoredVertex2F* ptr = vertices.data();
    for(Circle2D* circle : m_circles) {
        ptr->Position = circle->Position;
        ptr->Color = circle->Color;
        ptr++;
        Point2F radius = circle->Radius;
        for(double i(0); i < 2 * M_PI; i += M_PI / 6) { //<-- Change this Value
            Q_ASSERT(ptr != vertices.data() + verticesCount);
            ptr->Position = Point2F(cos(i) * radius.X(), sin(i) * radius.Y()) + circle->Position;
            ptr->Color = circle->Color;
            ptr++;
        }
        ptr->Position = Point2F(cos(0) * radius.X(), sin(0) * radius.Y()) + circle->Position;
        ptr->Color = circle->Color;
        ptr++;
    }

    m_buffer->UpdateVertexArray(vertices);
}
