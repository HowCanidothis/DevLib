#include "gtmeshcircle2D.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <qmath.h>

Circle2D::Circle2D(const Point2F& pos, const Point2F& radius, const Color3F& color)
    : position(pos)
    , radius(radius)
    , color(color)
{

}

GtMeshCircle2D::GtMeshCircle2D()
    : GtMeshBase(GL_POINTS)
{

}

void GtMeshCircle2D::resize(qint32 size)
{
    circles.resizeAndAllocate(size);
}

void GtMeshCircle2D::clear()
{
    circles.Clear();
}

Circle2D* GtMeshCircle2D::add(float x, float y, float r, const Color3F& color)
{
    Circle2D* result = new Circle2D(Point2F(x,y),Point2F(r,r), color);
    circles.InsertSortedUnique(result);
    return result;
}

void GtMeshCircle2D::remove(Circle2D* circle)
{
    delete circle;
    auto find = circles.FindSorted(circle);
    circles.Remove(find);
}

bool GtMeshCircle2D::buildMesh()
{
    if(!circles.Size())
        return false;
    qint32 vertices_per_circle = 12 + 1 + 1;
    vertices_count = vertices_per_circle * circles.Size();
    ScopedPointer<ColoredVertex2F> vertices(new ColoredVertex2F[vertices_count]);
    ColoredVertex2F* ptr = vertices.data();
    for(Circle2D* circle : circles) {
        ptr->position = circle->position;
        ptr->color = circle->color;
        ptr++;
        Point2F radius = circle->radius;
        for(double i(0); i < 2 * M_PI; i += M_PI / 6) { //<-- Change this Value
            Q_ASSERT(ptr != vertices.data() + vertices_count);
            ptr->position = Point2F(cos(i) * radius.x(), sin(i) * radius.y()) + circle->position;
            ptr->color = circle->color;
            ptr++;
        }
        ptr->position = Point2F(cos(0) * radius.x(), sin(0) * radius.y()) + circle->position;
        ptr->color = circle->color;
        ptr++;
    }
    vbo->bind();
    vbo->allocate(vertices.data(), vertices_count * sizeof(ColoredVertex2F));
    vbo->release();

    return true;
}

void GtMeshCircle2D::bindVAO(OpenGLFunctions* f)
{
    vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(ColoredVertex2F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex2F),(const void*)sizeof(Point2F));
}


