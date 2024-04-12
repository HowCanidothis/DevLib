#ifndef SHAREDMODULE_DECL_H
#define SHAREDMODULE_DECL_H

#include <SharedModule/internal.hpp>

#ifdef QT_GUI_LIB

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QDateTime>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
#include <QOpenGLFunctions>
typedef QOpenGLFunctions OpenGLFunctions;
#endif
typedef QQuaternion Quaternion;
typedef QPoint Point2I;

typedef QVector4D Point4F;
typedef Point4F Vector4F;
typedef QMatrix4x4 Matrix4;
typedef QSizeF SizeF;
typedef QSize SizeI;
typedef QRect RectI;

template<class T> inline static void point2DFromString(T& point, const QString& string){
    thread_local static QRegularExpression re(R"((\()?([^,]+),([^,\)]+)\)?)");
    auto m = re.match(string);
    if(m.isValid()) {
        point.X() = m.captured(2).toDouble();
        point.Y() = m.captured(3).toDouble();
    }

}
template<class T> inline static void point3DFromString(T& point, const QString& string){
    thread_local static QRegularExpression re(R"((\()?([^,]+),([^,]+),([^,\)]+)\)?)");
    auto m = re.match(string);
    if(m.isValid()) {
        point.X() = m.captured(2).toDouble();
        point.Y() = m.captured(3).toDouble();
        point.Z() = m.captured(4).toDouble();
    }
}

template<class T> inline static QString point2DToString(const T& point){ return QString("(" + QString::number(point.X()) + "," + QString::number(point.Y()) + ")"); }

template<class T> inline static QString point3DToString(const T& point){ return QString("(" + QString::number(point.X()) + "," + QString::number(point.Y()) + "," + QString::number(point.Z())  + ")"); }


#pragma pack(1)
class Point2D
{
    double m_x;
    double m_y;

public:
    Point2D()
        : m_x(0.0)
        , m_y(0.0)
    {}
    Point2D(double x, double y)
        : m_x(x)
        , m_y(y)
    {}

    double& X() { return m_x; }
    const double& X() const { return m_x; }

    double& Y() { return m_y; }
    const double& Y() const { return m_y; }

    bool operator <(const Point2D& other) const{
        if(!qFuzzyIsNull(X() - other.X())) return X() < other.X();
        return Y() < other.Y();
    }
    void FromString(const QString& v){ point2DFromString(*this,v); }
    QString ToString() const { return point2DToString(*this); }
};
Q_DECLARE_TYPEINFO(Point2D, Q_PRIMITIVE_TYPE);

struct Point3D : public Point2D
{
    double m_z;
public:
    Point3D(double x, double y, double z)
        : Point2D(x,y)
        , m_z(z)
    {}
    Point3D()
        : m_z(0.0)
    {}

    double& Z() { return m_z; }
    const double& Z() const { return m_z; }

    bool operator <(const Point3D& other) const{
        if(!qFuzzyIsNull(X() - other.X())) return X() < other.X();
        if(!qFuzzyIsNull(Y() - other.Y())) return Y() < other.Y();
        else return Z() < other.Z();
    }
    void FromString(const QString& v){ point3DFromString(*this,v); }
    QString ToString() const { return point3DToString(*this); }
};
Q_DECLARE_TYPEINFO(Point3D, Q_PRIMITIVE_TYPE);

class DateTime
{
   qint64 date;
   qint32 time;
public:
   void SetQDateTime(const QDateTime& dt){ this->SetQDate(dt.date()); this->SetQTime(dt.time()); }
   void SetQTime(const QTime& time){ this->time = time.msecsSinceStartOfDay(); }
   QTime ToQTime() const { return QTime::fromMSecsSinceStartOfDay(time); }
   void SetQDate(const QDate& date){ this->date = date.toJulianDay(); }
   QDate ToQDate() const { return QDate::fromJulianDay(date); }
   QDateTime ToQDateTime() const { return QDateTime(ToQDate(), ToQTime()); }
   qint64 GetDateValue() const { return date; }
   qint32 GetTimeValue() const { return time; }

   DateTime& operator =(const QDateTime& time) { SetQDateTime(time); return *this;  }
   QString ToString() const { return ToQDateTime().toString(); }
   void FromString(const QString& str){ this->SetQDateTime(QDateTime::fromString(str)); }
   bool operator<(const DateTime& other) const { return (date == other.date) ? (time < other.time) : (date < other.date); }
};
Q_DECLARE_TYPEINFO(DateTime, Q_PRIMITIVE_TYPE);

class Vector2F : public QVector2D
{
    typedef QVector2D Super;
public:
    Vector2F()
        : Super()
    {}
    Vector2F(const QVector2D&& another) // TODO. Bad performance, must be created own math library, or sometime c++ will allow us classes extending
        : Super(std::move(another))
    {}
    using Super::Super;

    float& X() { return operator[](0); }
    float X() const { return operator[](0); }

    float& Y() { return operator[](1); }
    float Y() const { return operator[](1); }

    Vector2F ortho() const { return Vector2F(-Y(), X()); }

    bool operator <(const Vector2F& other) const{
        if(!qFuzzyIsNull(X() - other.X())) return X() < other.X();
        return Y() < other.Y();
    }

    bool EqualTo(const Vector2F& another, float epsilon = std::numeric_limits<float>().epsilon()) const
    {
        return ::fuzzyCompare(another.x(), x(), epsilon) && ::fuzzyCompare(another.y(), y(), epsilon);
    }
    void FromString(const QString& v){ point2DFromString(*this,v); }
    QString ToString() const { return point2DToString(*this); }
};
Q_DECLARE_TYPEINFO(Vector2F, Q_PRIMITIVE_TYPE);

typedef Vector2F Point2F;

class Vector3F : public QVector3D
{
    typedef QVector3D Super;
public:
    using Super::Super;
    Vector3F()
        : Super()
    {}
    Vector3F(const QVector3D&& another) // TODO. Bad performance, must be created own math library, or sometime c++ will allow us classes extending
        : Super(std::move(another))
    {}

    static Vector3F& FromQVector3D(QVector3D& vector) { return reinterpret_cast<Vector3F&>(vector); }
    static const Vector3F& FromQVector3D(const QVector3D& vector) { return reinterpret_cast<const Vector3F&>(vector); }

    float& X() { return operator[](0); }
    float X() const { return operator[](0); }

    float& Y() { return operator[](1); }
    float Y() const { return operator[](1); }

    float& Z() { return operator[](2); }
    float Z() const { return operator[](2); }

    float Sum() const { return x() + y() + z(); }

    bool operator <(const Vector3F& other) const{
        if(!qFuzzyIsNull(x() - other.x())) return x() < other.x();
        if(!qFuzzyIsNull(y() - other.y())) return y() < other.y();
        else return z() < other.z();
    }

    bool EqualTo(const Vector3F& another, float epsilon = std::numeric_limits<float>().epsilon()) const
    {
        return ::fuzzyCompare(another.x(), x(), epsilon) && ::fuzzyCompare(another.y(), y(), epsilon) && ::fuzzyCompare(another.z(), z(), epsilon);
    }

    Vector3F normalized() const { return Super::normalized(); }

    float ProjectToLineSegmentT(const Vector3F& a, const Vector3F& b) const
    {
        auto ba = b - a;
        const float l2 = dotProduct(ba, ba);
        if (qFuzzyIsNull(l2)) {
            return 0.0;
        }
        auto t = dotProduct(*this - a, ba) / l2;
        return ::clamp(t, 0.f, 1.f);
    }

    float SquaredDistanceToPoint(const Vector3F& point) const
    {
        auto v = *this - point;
        return dotProduct(v,v);
    }

    Vector3F ProjectToLineSegment(const Vector3F& a, const Vector3F& b) const
    {
        return ::lerp(a, b, ProjectToLineSegmentT(a,b));
    }

    Vector3F ProjectToPlane(const Vector3F& planeNormal, const Vector3F& planePosition) const
    {
        const auto& p = *this;
        auto fullDirection = (p - planePosition);
        auto distToPlane = Vector3F::dotProduct(fullDirection, planeNormal);
        return p - distToPlane * planeNormal;
    }

    static std::tuple<Vector3F, float, Vector3F, float, float> DistanceBetweenLineSegments(const Vector3F& ap, const Vector3F& bp, const Vector3F& cp, const Vector3F& dp)
    {
        const double SMALL_NUM = 0.0000000001;
        auto u = bp - ap;
        auto v = dp - cp;
        auto w = ap - cp;

        double a = dotProduct(u, u);         // always >= 0
        double b = dotProduct(u, v);
        double c = dotProduct(v, v);         // always >= 0
        double d = dotProduct(u, w);
        double e = dotProduct(v, w);
        double D = a * c - b * b;        // always >= 0
        double sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
        double tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

        // compute the line parameters of the two closest points
        if (D < SMALL_NUM) { // the lines are almost parallel
            sN = 0.0;         // force using point P0 on segment S1
            sD = 1.0;         // to prevent possible division by 0.0 later
            tN = e;
            tD = c;
        } else {                 // get the closest points on the infinite lines
            sN = (b * e - c * d);
            tN = (a * e - b * d);
            if (sN < 0.0) {        // sc < 0 => the s=0 edge is visible
                sN = 0.0;
                tN = e;
                tD = c;
            } else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
                sN = sD;
                tN = e + b;
                tD = c;
            }
        }

        if (tN < 0.0) {            // tc < 0 => the t=0 edge is visible
            tN = 0.0;
            // recompute sc for this edge
            if (-d < 0.0) {
                sN = 0.0;
            } else if (-d > a) {
                sN = sD;
            } else {
                sN = -d;
                sD = a;
            }
        } else if (tN > tD) {      // tc > 1  => the t=1 edge is visible
            tN = tD;
            // recompute sc for this edge
            if ((-d + b) < 0.0) {
                sN = 0;
            } else if ((-d + b) > a) {
                sN = sD;
            } else {
                sN = (-d + b);
                sD = a;
            }
        }
        // finally do the division to get sc and tc
        sc = (abs(sN) < SMALL_NUM ? 0.0 : sN / sD);
        tc = (abs(tN) < SMALL_NUM ? 0.0 : tN / tD);

        auto point1 = ap + u * sc;
        auto point2 = cp + v * tc;

        // get the difference of the two closest points
        auto dP = (w + u * sc) - v * tc;  // =  S1(sc) - S2(tc)

        return std::make_tuple(point1, sc, point2, tc, dP.length());

        /*auto segA = ap, segC = cp, segB = bp, segD = dp;
        auto inPlaneA = segA.ProjectToPlane(segC, segD-segC);
        auto inPlaneB = segB.ProjectToPlane(segC, segD-segC);
        auto inPlaneBA = inPlaneB-inPlaneA;
        auto t = dotProduct(segC-inPlaneA, inPlaneBA)/dotProduct(inPlaneBA, inPlaneBA);
        t = !inPlaneA.EqualTo(inPlaneB) ? t : 0.f; // Zero's t if parallel
        auto segABtoLineCD = ::lerp(segA, segB, ::clamp(t, 0.f, 1.f));

        auto s = segABtoLineCD.ProjectToLineSegmentT(segC, segD);
        auto segCDtoSegAB = ::lerp(segC, segD, s);
        t = segCDtoSegAB.ProjectToLineSegmentT(segA, segB);
        auto segABtoSegCD = ::lerp(segA, segB, t);
        return std::make_tuple(segABtoSegCD, t, segCDtoSegAB, s, segABtoSegCD.distanceToPoint(segCDtoSegAB));*/

        /*{
            auto cd = d - c;
            float lineDirSqrMag = dotProduct(cd, cd);
            auto inPlaneA = a - ((dotProduct(a-c, cd)/lineDirSqrMag)*cd);
            auto inPlaneB = b - ((dotProduct(b-c, cd)/lineDirSqrMag)*cd);
            auto inPlaneAB = inPlaneB - inPlaneA;
            auto t = dotProduct(c - inPlaneA, inPlaneA) / dotProduct(inPlaneAB, inPlaneAB);
            t = inPlaneA.EqualTo(inPlaneB) ? 0.f : t;
            auto abProjection = ::lerp(a,b,::clamp(t, 0.f, 1.f));
            auto s = abProjection.ProjectToLineSegmentT(c,d);
            auto cdPoint = ::lerp(c, d, s);
            t = cdPoint.ProjectToLineSegmentT(a,b);
            auto abPoint = ::lerp(a, b, t);
        }*/

    }

    Vector3F operator+(const Vector3F& another) const { return static_cast<Vector3F>((toBase() + another.toBase())); }
    Vector3F operator-(const Vector3F& another) const { return static_cast<Vector3F>((toBase() - another.toBase())); }
    Vector3F operator*(const Vector3F& another) const { return static_cast<Vector3F>((toBase() * another.toBase())); }
    Vector3F operator*(float value) const { return static_cast<Vector3F>((toBase() * value)); }
    Vector3F operator/(float value) const { return static_cast<Vector3F>((toBase() / value)); }

    friend Vector3F operator*(float value, const Vector3F& another) { return static_cast<Vector3F>(value * static_cast<const QVector3D&>(another)); }

    void FromString(const QString& v){ point3DFromString(*this,v); }
    QString ToString() const { return point3DToString(*this); }

private:
    const QVector3D& toBase() const { return *this; }
};
Q_DECLARE_TYPEINFO(Vector3F, Q_PRIMITIVE_TYPE);
typedef Vector3F Point3F;
Q_DECLARE_METATYPE(Vector3F);

class Vector3D
{
public:
    Vector3D()
        : m_x(0.0)
        , m_y(0.0)
        , m_z(0.0)
    {}
    Vector3D(double x, double y, double z)
        : m_x(x)
        , m_y(y)
        , m_z(z)
    {}

    double& X() { return m_x; }
    double X() const { return m_x; }

    double& Y() { return m_y; }
    double Y() const { return m_y; }

    double& Z() { return m_z; }
    double Z() const { return m_z; }

    Vector3D operator-() const
    {
        return Vector3D(-m_x, -m_y, -m_z);
    }

    Vector3D operator-(const Vector3D& another) const
    {
        return Vector3D(m_x - another.m_x, m_y - another.m_y, m_z - another.m_z);
    }

    Vector3D operator+(const Vector3D& another) const
    {
        return Vector3D(m_x + another.m_x, m_y + another.m_y, m_z + another.m_z);
    }

    bool EqualTo(const Vector3D& another, double epsilon = std::numeric_limits<double>().epsilon()) const
    {
        return ::fuzzyCompare(another.X(), X(), epsilon) && ::fuzzyCompare(another.Y(), Y(), epsilon) && ::fuzzyCompare(another.Z(), Z(), epsilon);
    }

    bool operator==(const Vector3D& another) const
    {
        return EqualTo(another);
    }

    Vector3F To3F() const { return Vector3F(m_x, m_y, m_z); }

private:
    double m_x,m_y,m_z;
};

#pragma pack()

class BoundingRect
{
    Point2F m_left;
    Point2F m_right;
public:
    enum UnknownPoints{
        FromUnknownPoints
    };

    BoundingRect() {}
    BoundingRect(const Point2F& left, const Point2F& right) : m_left(left), m_right(right) {}
    BoundingRect(const Point2F& left, const Point2F& right, UnknownPoints);

    float Left() const { return m_left.x(); }
    float Top() const { return m_left.y(); }
    float Right() const { return m_right.x(); }
    float Bottom() const { return m_right.y(); }

    const Point2F& GetLeft() const { return m_left; }
    const Point2F& GetRight() const { return m_right; }
    float Width() const { return GetWidth(); }
    float Height() const { return GetHeight(); }
    float GetWidth() const { return m_right.X() - m_left.X(); }
    float GetHeight() const { return m_right.Y() - m_left.Y(); }
    void Extend(float width, float height)
    {
        auto wh = width / 2.f;
        auto hh = height/ 2.f;
        m_left.X() -= wh; m_right.X() += wh;
        m_left.Y() -= hh; m_right.Y() += hh;
    }
    Point2F GetCenter() const { return (m_right + m_left) / 2.f; }

    bool IsInf() const { return qIsInf(m_left.x()) || qIsInf(m_left.y()) || qIsInf(m_right.x()) || qIsInf(m_right.y()); }

    bool Intersects(const BoundingRect& other) const;

    BoundingRect& Unite(const BoundingRect& another)
    {
        const auto& tl = another.GetLeft();
        const auto& br = another.GetRight();
        if(tl.x() < m_left.x()) {
            m_left.setX(tl.x());
        }
        if(tl.y() < m_left.y()) {
            m_left.setY(tl.y());
        }

        if(br.x() > m_right.x()) {
            m_right.setX(br.x());
        }
        if(br.y() > m_right.y()) {
            m_right.setY(br.y());
        }
        return *this;
    }

    bool operator!= (const BoundingRect& another) const
    {
        return !m_left.EqualTo(another.m_left) || !m_right.EqualTo(another.m_right);
    }
    bool operator== (const BoundingRect& another) const
    {
        return !operator!=(another);
    }
};

class BoundingSphere
{
    Point3F m_position;
    float m_radius;
public:
    BoundingSphere()
        : m_radius(0.f)
    {}

    BoundingSphere(const Vector3F& position, float radius)
        : m_position(position)
        , m_radius(radius)
    {}

    void SetPosition(const Point3F& position)
    {
        m_position = position;
    }

    const Point3F& GetPosition() const { return m_position; }

    void SetRadius(float radius)
    {
        m_radius = radius;
    }

    float GetRadius() const { return m_radius; }
};

class BoundingBox
{
    Point3F m_topLeftFront;
    Point3F m_bottomRightBack;
public:
    enum UnknownPoints{
        FromUnknownPoints
    };
    BoundingBox() {}
    BoundingBox(float centerX, float centerY, float centerZ, float w, float h, float d)
        : m_topLeftFront(centerX - w / 2.f, centerY + h / 2.f, centerZ + d / 2.f)
        , m_bottomRightBack(centerX + w / 2.f, centerY - h / 2.f, centerZ - d / 2.f)
    {}
    BoundingBox(const Point3F& left, const Point3F& right) :
        m_topLeftFront(left),
        m_bottomRightBack(right)
    {}
    BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, bool)
        : m_topLeftFront(minX, maxY, maxZ)
        , m_bottomRightBack(maxX, minY, minZ)
    {}
    BoundingBox(const Point3F p1, const Point3F& p2, UnknownPoints)
        : BoundingBox(p1.x(), p2.x(), p1.y(), p2.y(), p1.z(), p2.z(), FromUnknownPoints)
    {}
    BoundingBox(float x1, float x2, float y1, float y2, float z1, float z2, UnknownPoints)
    {
        float xMin, xMax, yMin, yMax, zMin, zMax;
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

        if(z1 < z2) {
            zMin = z1;
            zMax = z2;
        } else {
            zMin = z2;
            zMax = z1;
        }
        m_topLeftFront = Point3F(xMin, yMax, zMax);
        m_bottomRightBack = Point3F(xMax, yMin, zMin);
    }

    void SetNull()
    {
        m_topLeftFront = m_bottomRightBack;
    }

    void MultiplyExtent(float multiplier)
    {
        auto currentExtent = CalculateExtent();
        AddExtent(currentExtent.x() * multiplier, currentExtent.y() * multiplier, currentExtent.z() * multiplier);
    }
    void AddExtent(float x, float y, float z)
    {
        auto xh = x / 2.f, yh = y / 2.f, zh = z / 2.f;
        m_topLeftFront.X() -= xh;
        m_topLeftFront.Y() += yh;
        m_topLeftFront.Z() += zh;
        m_bottomRightBack.X() += xh;
        m_bottomRightBack.Y() -= yh;
        m_bottomRightBack.Z() -= zh;
    }

    float Width() const { return m_bottomRightBack.x() - m_topLeftFront.x(); }
    float Height() const { return m_topLeftFront.y() - m_bottomRightBack.y(); }
    float Depth() const { return m_topLeftFront.z() - m_bottomRightBack.z(); }
    Point3F CalculateExtent() const { return Point3F(m_bottomRightBack.x() - m_topLeftFront.x(), m_topLeftFront.y() - m_bottomRightBack.y(), m_topLeftFront.z() - m_bottomRightBack.z()); }
    Point3F Center() const { return (m_topLeftFront + m_bottomRightBack) / 2.f; }
    const Point3F& GetLeft() const { return m_topLeftFront; }
    const Point3F& GetRight() const { return m_bottomRightBack; }
    float X() const { return m_topLeftFront.x(); }
    float Y() const { return m_topLeftFront.y(); }
    float Right() const { return m_bottomRightBack.x(); }
    float Top() const { return m_topLeftFront.y(); }
    float Left() const { return m_topLeftFront.x(); }
    float Bottom() const { return m_bottomRightBack.y(); }
    float Front() const { return m_topLeftFront.z(); }
    float Back() const { return m_bottomRightBack.z(); }

    float GetMinX() const { return m_topLeftFront.x(); }
    float GetMinY() const { return m_bottomRightBack.y(); }
    float GetMinZ() const { return m_bottomRightBack.z(); }
    float GetMaxX() const { return m_bottomRightBack.x(); }
    float GetMaxY() const { return m_topLeftFront.y(); }
    float GetMaxZ() const { return m_topLeftFront.z(); }

    float& MinX() { return m_topLeftFront.X(); }
    float& MinY() { return m_bottomRightBack.Y(); }
    float& MinZ() { return m_bottomRightBack.Z(); }
    float& MaxX() { return m_bottomRightBack.X(); }
    float& MaxY() { return m_topLeftFront.Y(); }
    float& MaxZ() { return m_topLeftFront.Z(); }

    bool Intersects(const BoundingBox& other) const
    {
        return !(GetMaxX() < other.GetMinX() || GetMinX() > other.GetMaxX() ||
                 GetMaxY() < other.GetMinY() || GetMinY() > other.GetMaxY() ||
                 GetMaxZ() < other.GetMinZ() || GetMinZ() > other.GetMaxZ()
                 );
    }
    BoundingBox& Unite(const BoundingBox& other)
    {
        const auto& otl = other.GetLeft();
        const auto& obr = other.GetRight();
        if(otl.x() < m_topLeftFront.x()) {
            m_topLeftFront.setX(otl.x());
        }
        if(otl.y() > m_topLeftFront.y()) {
            m_topLeftFront.setY(otl.y());
        }
        if(otl.z() > m_topLeftFront.z()) {
            m_topLeftFront.setZ(otl.z());
        }

        if(obr.x() > m_bottomRightBack.x()) {
            m_bottomRightBack.setX(obr.x());
        }
        if(obr.y() < m_bottomRightBack.y()) {
            m_bottomRightBack.setY(obr.y());
        }
        if(obr.z() < m_bottomRightBack.z()) {
            m_bottomRightBack.setZ(obr.z());
        }
        return *this;
    }

    BoundingBox Translated(const Point3F& translation) const { return BoundingBox(m_topLeftFront + translation, m_bottomRightBack + translation); }
    BoundingBox Transformed(const QMatrix4x4 transform) const
    {
        Point3F p1 = transform * (m_topLeftFront);
        Point3F p2 = transform * (m_bottomRightBack);
        return BoundingBox(p1, p2, FromUnknownPoints);
    }

    bool operator!=(const BoundingBox& another) const
    {
        return !GetLeft().EqualTo(another.GetLeft()) || !GetRight().EqualTo(another.GetRight());
    }

    bool operator==(const BoundingBox& another) const
    {
        return !operator!=(another);
    }

    bool IsNull() const { return m_bottomRightBack == m_topLeftFront; }
};

typedef quint32 gTexID;
typedef quint32 gLocID;
typedef quint32 gTexUnit;
typedef quint32 gTexTarget;
typedef quint32 gTexInternalFormat;
typedef quint32 gPixFormat;
typedef quint32 gPixType;
typedef quint32 gRenderType;
typedef quint32 gIndicesType;
typedef quint32 gFboID;
typedef quint32 gRenderbufferID;

typedef Point3F Color3F;

inline Color3F Color3FCreate(const QColor& color) { return Color3F(color.redF(), color.greenF(), color.blueF()); }

#pragma pack(1)
struct TexturedVertex2F
{
    Point2F Position;
    Point2F TexCoord;
};

struct TexturedVertex3F
{
    Point3F Position;
    Point2F TexCoord;
};

struct Vertex3f3f
{
    Point3F Position;
    Point3F Normal;
};

struct Vertex3f2f2f
{
    Point3F Position;
    Point2F A;
    Point2F B;
};

struct ColoredVertex2F
{
    Point2F Position;
    Color3F Color;

    ColoredVertex2F()
    {}
    ColoredVertex2F(const Point2F& pos, const Color3F& color)
        : Position(pos)
        , Color(color)
    {}
};

struct ColoredVertex3F
{
    Point3F Position;
    Color3F Color;
};

struct StatedVertex3F
{
    Point3F Position;
    quint8 State;
    quint8 Transparency;

    StatedVertex3F()
        : State(0)
        , Transparency(255)
    {}

    StatedVertex3F(float x, float y, float z, quint8 state = 0, quint8 transparency = 255)
        : Position(x,y,z)
        , State(state)
        , Transparency(transparency)
    {}

    StatedVertex3F(const Point3F& point, quint8 state = 0, quint8 transparency = 255)
        : StatedVertex3F(point.x(), point.y(), point.z(), state, transparency)
    {}

    bool operator==(const StatedVertex3F& another) const { return another.State == State && another.Position == Position; }
};

#pragma pack()


#define GT_CONTENT_PATH "../../../Content/"
#define GT_STYLES_PATH GT_CONTENT_PATH "CSS/"
#define GT_SHADERS_PATH ":/"

#endif // QT_GUI_LIB

#endif // GT_DECL_H
