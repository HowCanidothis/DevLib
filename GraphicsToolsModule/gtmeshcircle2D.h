#ifndef GTMESHCIRCLE2D_H
#define GTMESHCIRCLE2D_H

#include "gtmeshbase.h"
#include "SharedModule/array.h"

struct Circle2D
{
    Point2F Position;
    Point2F Radius;
    Color3F Color;

    Circle2D(){}
    Circle2D(const Point2F& pos, const Point2F& Radius, const Color3F& Color);
};

class GtMeshCircle2D : public GtMeshBase
{
    ArrayPointers<Circle2D> m_circles;
public:
    GtMeshCircle2D();

    void Resize(qint32 size);
    void Clear();
    Circle2D* Add(float x, float y, float r, const Color3F& color);
    void Remove(Circle2D* circle);

    ArrayPointers<Circle2D>::iterator Begin() { return m_circles.Begin(); }
    ArrayPointers<Circle2D>::iterator End() { return m_circles.End(); }
    ArrayPointers<Circle2D>::const_iterator Begin() const { return m_circles.Begin(); }
    ArrayPointers<Circle2D>::const_iterator End() const { return m_circles.End(); }

    // GtMeshBase interface
protected:
    bool buildMesh() Q_DECL_OVERRIDE;
    void bindVAO(OpenGLFunctions* f) Q_DECL_OVERRIDE;
};

#endif // GTMESHCIRCLE_H
