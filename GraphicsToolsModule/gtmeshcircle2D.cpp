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
    : GtMeshBase(GL_POINTS)
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

bool GtMeshCircle2D::buildMesh()
{
    if(!m_circles.Size())
        return false;
    qint32 vertices_per_circle = 12 + 1 + 1;
    m_verticesCount = vertices_per_circle * m_circles.Size();
    ScopedPointer<ColoredVertex2F> vertices(new ColoredVertex2F[m_verticesCount]);
    ColoredVertex2F* ptr = vertices.data();
    for(Circle2D* circle : m_circles) {
        ptr->Position = circle->Position;
        ptr->Color = circle->Color;
        ptr++;
        Point2F radius = circle->Radius;
        for(double i(0); i < 2 * M_PI; i += M_PI / 6) { //<-- Change this Value
            Q_ASSERT(ptr != vertices.data() + m_verticesCount);
            ptr->Position = Point2F(cos(i) * radius.X(), sin(i) * radius.Y()) + circle->Position;
            ptr->Color = circle->Color;
            ptr++;
        }
        ptr->Position = Point2F(cos(0) * radius.X(), sin(0) * radius.Y()) + circle->Position;
        ptr->Color = circle->Color;
        ptr++;
    }
    m_vbo->bind();
    m_vbo->allocate(vertices.data(), m_verticesCount * sizeof(ColoredVertex2F));
    m_vbo->release();

    return true;
}

void GtMeshCircle2D::bindVAO(OpenGLFunctions* f)
{
    m_vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(ColoredVertex2F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex2F),(const void*)sizeof(Point2F));
}


