#ifndef CUBETREE_DECL_H
#define CUBETREE_DECL_H

#include <Qt>

#define CUBETREE_NAMESPACE CubeTree
#define CUBETREE_BEGIN_NAMESPACE namespace CUBETREE_NAMESPACE {
#define CUBETREE_END_NAMESPACE }
#define CUBETREE_USING_NAMESPACE using namespace CUBETREE_NAMESPACE;
#define CUBETREE_USE(V) CUBETREE_NAMESPACE::V
#define CUBETREE_DECL_FORWARD(template_name) \
    CUBETREE_BEGIN_NAMESPACE \
    template<typename template_name,qint32 v> class Tree; \
    CUBETREE_END_NAMESPACE

CUBETREE_BEGIN_NAMESPACE

enum Branch_Location
{
    Branch_TopLeftFront,
    Branch_TopRightFront,
    Branch_BottomRightFront,
    Branch_BottomLeftFront,
    Branch_TopLeftBack,
    Branch_TopRightBack,
    Branch_BottomRightBack,
    Branch_BottomLeftBack,
};

typedef float real;

enum BoundingBox_Location{
    LeftLessZero=0x1,
    RightLessZero=0x2,
    BottomLessZero=0x4,
    TopLessZero=0x8,
    FrontLessZero = 0x10,
    BackLessZero = 0x20,

    Location_TopLeftBack = LeftLessZero | RightLessZero | FrontLessZero | BackLessZero,
    Location_TopRightBack = FrontLessZero | BackLessZero,
    Location_BottomLeftBack = LeftLessZero | RightLessZero | BottomLessZero | TopLessZero | FrontLessZero | BackLessZero,
    Location_BottomRightBack = BottomLessZero | TopLessZero | FrontLessZero | BackLessZero,
    Location_MiddleTopBack = LeftLessZero | FrontLessZero | BackLessZero,
    Location_MiddleLeftBack = BottomLessZero | LeftLessZero | RightLessZero | FrontLessZero | BackLessZero,
    Location_MiddleRightBack = BottomLessZero | FrontLessZero | BackLessZero,
    Location_MiddleBottomBack = LeftLessZero | BottomLessZero | TopLessZero | FrontLessZero | BackLessZero,
    Location_CenterBack = LeftLessZero | BottomLessZero | FrontLessZero | BackLessZero,
    Location_Center = LeftLessZero | BottomLessZero | BackLessZero,
    Location_CenterFront = LeftLessZero | BottomLessZero,
    Location_TopLeftFront = LeftLessZero | RightLessZero,
    Location_TopRightFront = 0,
    Location_BottomLeftFront = LeftLessZero | RightLessZero | BottomLessZero | TopLessZero,
    Location_BottomRightFront = BottomLessZero | TopLessZero,
    Location_MiddleTopFront = LeftLessZero,
    Location_MiddleLeftFront = BottomLessZero | LeftLessZero | RightLessZero,
    Location_MiddleRightFront = BottomLessZero,
    Location_MiddleBottomFront = BottomLessZero | TopLessZero | LeftLessZero,
    Location_RightHalf = BottomLessZero | BackLessZero,
    Location_TopHalf = LeftLessZero | BackLessZero,
    Location_LeftHalf = LeftLessZero | RightLessZero | BottomLessZero | BackLessZero,
    Location_BottomHalf = LeftLessZero | TopLessZero | BottomLessZero | BackLessZero,
    Location_TopLeftQuart = LeftLessZero | RightLessZero | BackLessZero,
    Location_TopRightQuart = BackLessZero,
    Location_BottomLeftQuart = LeftLessZero | RightLessZero | TopLessZero | BottomLessZero | BackLessZero,
    Location_BottomRightQuart = TopLessZero | BottomLessZero | BackLessZero,
};


struct Point{
    real X;
    real Y;
    real Z;

    Point(real x = 0, real y = 0, real z = 0) : X(x), Y(y), Z(z) {}

    real SquaredDistanceToPoint(const Point& p) const { return SquaredDistanceToPoint(p.X, p.Y, p.Z); }
    real SquaredDistanceToPoint(real X, real Y, real z) const;
    real DistanceToPoint(const Point& p) const { return sqrt(SquaredDistanceToPoint(p)); }
    real DistanceToPoint(real x, real y, real z) const { return sqrt(SquaredDistanceToPoint(x,y,z)); }
    Point operator +(const Point& another) const { return Point(this->X + another.X, this->Y + another.Y, this->Z + another.Z); }
    Point operator/(real v) const { return Point(this->X / v, this->Y / v, this->Z / v);  }
};

struct BoundingBox
{
    Point TopLeftFront;
    Point BottomRightBack;

    BoundingBox(){}
    BoundingBox(real centerX, real centerY, real centerZ, real w, real h, real d) : TopLeftFront(Point(centerX - w / 2, centerY + h / 2, centerZ + d / 2)), BottomRightBack(Point(centerX + w / 2, centerY - h / 2, centerZ - d / 2)) {}
    BoundingBox(const Point& tlf, const Point& brb) : TopLeftFront(tlf), BottomRightBack(brb) {}
    static BoundingBox FromUnknownPoints(real x1, real y1, real z1, real x2, real y2, real z2);
    BoundingBox_Location LocationOfAnother(const BoundingBox& insideBox) const;
    bool Intersects(const BoundingBox& other) const;
    bool Contains(const BoundingBox& other) const;
    bool Contains(real x, real y, real z) const;
    bool Contains(const Point& p) const { return Contains(p.X,p.Y,p.Z); }
    real Left() const { return TopLeftFront.X; }
    real Top() const { return TopLeftFront.Y; }
    real Bottom() const { return BottomRightBack.Y; }
    real Right() const { return BottomRightBack.X; }
    real Back() const { return BottomRightBack.Z; }
    real Front() const { return TopLeftFront.Z; }
    Point GetCenter() const { return (TopLeftFront + BottomRightBack) / 2; }
    const Point& GetTopLeftFront() const { return TopLeftFront; }
    const Point& GetBottomRightBack() const { return BottomRightBack; }
    real GetWidth() const { return BottomRightBack.X - TopLeftFront.X; }
    real GetHeight() const { return TopLeftFront.Y - BottomRightBack.Y; }
    real GetDepth() const { return TopLeftFront.Z - BottomRightBack.Z; }
};

inline BoundingBox BoundingBox::FromUnknownPoints(real x1, real y1, real z1, real x2, real y2, real z2)
{
    real xmin, ymin, xmax, ymax, zmax, zmin;
    if(x1 < x2){
        xmin = x1;
        xmax = x2;
    } else {
        xmin = x2;
        xmax = x1;
    }
    if(y1 < y2){
        ymin = y1;
        ymax = y2;
    } else {
        ymin = y2;
        ymax = y1;
    }
    if(z1 < z2) {
        zmin = z1;
        zmax = z2;
    } else {
        zmin = z2;
        zmax = z1;
    }
    return BoundingBox(Point(xmin, ymax, zmax), Point(xmax, ymin, zmin));
}

inline BoundingBox_Location BoundingBox::LocationOfAnother(const BoundingBox& insideBox) const
{
    qint32 result(0);
    Point cen = GetCenter();
    result |= ((insideBox.Left() - cen.X) < 0) ? LeftLessZero : 0;
    result |= ((insideBox.Right() - cen.X) < 0) ? RightLessZero : 0;
    result |= ((insideBox.Top() - cen.Y) < 0) ? TopLessZero : 0;
    result |= ((insideBox.Bottom() - cen.Y) < 0) ? BottomLessZero : 0;
    result |= ((insideBox.Front() - cen.Z) < 0) ? FrontLessZero : 0;
    result |= ((insideBox.Back() - cen.Z) < 0) ? BackLessZero : 0;
    return (BoundingBox_Location)result;
}

inline bool BoundingBox::Intersects(const BoundingBox& another) const
{
    return this->Left() <= another.Right() && another.Left() <= this->Right()
            && this->Bottom() <= another.Top() && another.Bottom() <= this->Top()
            && this->Back() <= another.Front() && another.Back() <= this->Front();
}

inline bool BoundingBox::Contains(const BoundingBox& another) const
{
    return this->Left() <= another.Left() && this->Right() >= another.Right()
           && this->Top() >= another.Top() && this->Bottom() <= another.Bottom()
           && this->Front() >= another.Front() && this->Back() <= another.Back();
}

inline bool BoundingBox::Contains(real x, real y, real z) const
{
    return x > Left() && x < Right() &&
           y > Bottom() && y < Top() &&
           z > Back() && z < Front();
}

inline real Point::SquaredDistanceToPoint(real x, real y, real z) const
{
    real dX = x - this->X;
    real dY = y - this->Y;
    real dZ = z - this->Z;
    return dX * dX + dY * dY + dZ * dZ;
}

CUBETREE_END_NAMESPACE

#endif // QUADTREE_DECL_H
