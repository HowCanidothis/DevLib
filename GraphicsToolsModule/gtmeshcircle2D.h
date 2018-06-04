#ifndef GTMESHCIRCLE2D_H
#define GTMESHCIRCLE2D_H

#include "gtmeshbase.h"
#include "SharedModule/array.h"

struct Circle2D
{
    Point2F position;
    Point2F radius;
    Color3F color;

    Circle2D(){}
    Circle2D(const Point2F& pos, const Point2F& radius, const Color3F& color);
};

class GtMeshCircle2D : public GtMeshBase
{
    ArrayPointers<Circle2D> circles;
public:
    GtMeshCircle2D();

    void resize(qint32 size);
    void clear();
    Circle2D* add(float x, float y, float r, const Color3F& color);
    void remove(Circle2D* circle);

    ArrayPointers<Circle2D>::iterator begin() { return circles.Begin(); }
    ArrayPointers<Circle2D>::iterator end() { return circles.End(); }
    ArrayPointers<Circle2D>::const_iterator begin() const { return circles.Begin(); }
    ArrayPointers<Circle2D>::const_iterator end() const { return circles.End(); }
    // GtMeshBase interface
protected:
    bool buildMesh() Q_DECL_OVERRIDE;
    void bindVAO(OpenGLFunctions* f) Q_DECL_OVERRIDE;
};

#endif // GTMESHCIRCLE_H
