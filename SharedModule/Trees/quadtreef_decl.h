#ifndef QUADTREEF_DECL_H
#define QUADTREEF_DECL_H

#include <Qt>

#include <math.h>

#define QUADTREE_NAMESPACE QuadTreeF
#define QUADTREE_BEGIN_NAMESPACE namespace QUADTREE_NAMESPACE {
#define QUADTREE_END_NAMESPACE }
#define QUADTREE_USING_NAMESPACE using namespace QUADTREE_NAMESPACE;
#define QUADTREE_USE(V) QUADTREE_NAMESPACE::V
#define QUADTREE_DECL_FORWARD(template_name) \
    QUADTREE_BEGIN_NAMESPACE \
    template<typename template_name,qint32 v> class Tree; \
    QUADTREE_END_NAMESPACE




QUADTREE_BEGIN_NAMESPACE

enum Branch_Location
{
    Branch_TopLeft,
    Branch_TopRight,
    Branch_BottomRight,
    Branch_BottomLeft,
};

typedef float real;

enum BoundingRect_Location{
    LeftLessZero=0x1,
    RightLessZero=0x2,
    BottomLessZero=0x4,
    TopLessZero=0x8,

    Location_TopLeft = LeftLessZero | RightLessZero | BottomLessZero | TopLessZero,
    Location_TopRight = BottomLessZero | TopLessZero,
    Location_BottomLeft = LeftLessZero | RightLessZero,
    Location_BottomRight=0,
    Location_MiddleTop = BottomLessZero | TopLessZero | LeftLessZero,
    Location_MiddleLeft = TopLessZero | LeftLessZero | RightLessZero,
    Location_MiddleRight = TopLessZero,
    Location_MiddleBottom = LeftLessZero,
    Location_Center = LeftLessZero | TopLessZero
};


struct Point{
    real x;
    real y;

    Point(real _x=0, real _y=0) : x(_x), y(_y) {}

    real squaredDistanceToPoint(const Point& p) const { return squaredDistanceToPoint(p.x, p.y); }
    real squaredDistanceToPoint(real x, real y) const;
    real distanceToPoint(const Point& p) const { return sqrt(squaredDistanceToPoint(p)); }
    real distanceToPoint(real x, real y) const { return sqrt(squaredDistanceToPoint(x,y)); }
    Point operator +(const Point& other) const { return Point(this->x + other.x, this->y + other.y); }
    Point operator/(real v) const { return Point(this->x / v, this->y / v);  }
};

struct BoundingRect
{
    enum InitializationMode {
        Extent
    };

    Point topLeft;
    Point bottomRight;

    BoundingRect(){}
    BoundingRect(real xCenter, real yCenter, real extentX, real extentY, InitializationMode)
        : topLeft(Point(xCenter - extentX, yCenter - extentY))
        , bottomRight(Point(xCenter + extentX, yCenter + extentY))
    {}
    BoundingRect(real xCenter, real yCenter, real w, real h) : topLeft(Point(xCenter - w / 2.f, yCenter - h / 2.f)), bottomRight(Point(xCenter + w / 2.f, yCenter + h / 2.f)) {}
    BoundingRect(const Point& tl, const Point& br) : topLeft(tl), bottomRight(br) {}
    static BoundingRect fromUnknownPoints(real x1, real y1, real x2, real y2);
    BoundingRect_Location locationOfOther(const BoundingRect &insideRect) const;
    bool intersects(const BoundingRect& other) const;
    bool contains(const BoundingRect& other) const;
    bool contains(real x, real y) const;
    bool contains(const Point& p) const { return contains(p.x,p.y); }
    real left() const { return topLeft.x; }
    real top() const { return topLeft.y; }
    real bottom() const { return bottomRight.y; }
    real right() const { return bottomRight.x; }
    Point getCenter() const { return (topLeft + bottomRight) / 2; }
    Point getTopRight() const { return Point(bottomRight.x, topLeft.y); }
    Point getBottomLeft() const { return Point(topLeft.x, bottomRight.y); }
    const Point& getTopLeft() const { return topLeft; }
    const Point& getBottomRight() const { return bottomRight; }
    real getWidth() const { return bottomRight.x - topLeft.x; }
    real getHeight() const { return bottomRight.y - topLeft.y; }
};

inline BoundingRect BoundingRect::fromUnknownPoints(real x1, real y1, real x2, real y2)
{
    real xmin, ymin, xmax, ymax;
    if(x1 < x2){
        xmin = x1;
        xmax = x2;
    }
    else{
        xmin = x2;
        xmax = x1;
    }
    if(y1 < y2){
        ymin = y1;
        ymax = y2;
    }
    else{
        ymin = y2;
        ymax = y1;
    }
    return BoundingRect(Point(xmin, ymin), Point(xmax, ymax));
}

inline BoundingRect_Location BoundingRect::locationOfOther(const BoundingRect& insideRect) const
{
    qint32 d(0);
    Point cen = getCenter();
    d |= ((insideRect.left() - cen.x) > 0) ? 0 : LeftLessZero;
    d |= ((insideRect.right() - cen.x) > 0) ? 0 : RightLessZero;
    d |= ((insideRect.top() - cen.y) > 0) ? 0 : TopLessZero;
    d |= ((insideRect.bottom() - cen.y) > 0) ? 0 : BottomLessZero;
    return (BoundingRect_Location)d;
}

inline bool BoundingRect::intersects(const BoundingRect &other) const
{
    return this->left()<=other.right() && other.left() <= this->right()
            && this->top()<=other.bottom() && other.top()<=this->bottom();
}

inline bool BoundingRect::contains(const BoundingRect &other) const
{
    return this->left()<= other.left() && this->right() >= other.right() && this->top() <= other.top() && this->bottom() >= other.bottom();
}

inline bool BoundingRect::contains(real x, real y) const
{
    return (x > this->topLeft.x) &&  (x < this->bottomRight.x) &&
            (y > this->topLeft.y) &&  (y < this->bottomRight.y);
}

inline real Point::squaredDistanceToPoint(real x, real y) const
{
    real lX = x - this->x;
    real lY = y - this->y;
    return lX * lX + lY * lY;
}

QUADTREE_END_NAMESPACE

#endif // QUADTREE_DECL_H
