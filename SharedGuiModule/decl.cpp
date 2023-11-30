#include "decl.h"

BoundingRect::BoundingRect(const Point2F& left, const Point2F& right, UnknownPoints)
{
    float xMin, xMax, yMin, yMax;
    auto x1 = left.x();
    auto x2 = right.x();
    auto y1 = left.y();
    auto y2 = right.y();
    if(x1 < x2) {
        xMin = x1;
        xMax = x2;
    } else {
        xMin = x2;
        xMax = x1;
    }

    if(y1 < y2) {
        yMin = y1;
        yMax = y2;
    } else {
        yMin = y2;
        yMax = y1;
    }

    m_left = Point2F(xMin, yMin);
    m_right = Point2F(xMax, yMax);
}

bool BoundingRect::Intersects(const BoundingRect& other) const
{
    return !(m_right.X() < other.m_left.X() || m_left.X() > other.m_right.X() ||
             m_right.Y() < other.m_left.Y() || m_left.Y() > other.m_right.Y()
             );
}
