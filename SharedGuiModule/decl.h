#ifndef SHAREDMODULE_DECL_H
#define SHAREDMODULE_DECL_H

#include <SharedModule/internal.hpp>

#ifdef QT_GUI_LIB

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QDateTime>
#include <QOpenGLFunctions_4_5_Core>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
typedef QOpenGLFunctions_4_5_Core OpenGLFunctions;
#endif
typedef QQuaternion Quaternion;
typedef QPoint Point2I;

typedef QVector4D Point4F;
typedef Point4F Vector4F;
typedef QMatrix4x4 Matrix4;
typedef QSizeF SizeF;
typedef QSize SizeI;
typedef QRect Rect;

template<class T> inline static void point2DFromString(T& point, const QString& string){
    static QRegExp re(R"((\()?([^,]+),([^,\)]+)\)?)");
    re.indexIn(string);
    point.X() = re.cap(2).toDouble();
    point.Y() = re.cap(3).toDouble();
}
template<class T> inline static void point3DFromString(T& point, const QString& string){
    static QRegExp re(R"((\()?([^,]+),([^,]+),([^,\)]+)\)?)");
    re.indexIn(string);
    point.X() = re.cap(2).toDouble();
    point.Y() = re.cap(3).toDouble();
    point.Z() = re.cap(4).toDouble();
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

    float& X() { return operator[](0); }
    float X() const { return operator[](0); }

    float& Y() { return operator[](1); }
    float Y() const { return operator[](1); }

    float& Z() { return operator[](2); }
    float Z() const { return operator[](2); }

    bool operator <(const Vector3F& other) const{
        if(!qFuzzyIsNull(x() - other.x())) return x() < other.x();
        if(!qFuzzyIsNull(y() - other.y())) return y() < other.y();
        else return z() < other.z();
    }
    void FromString(const QString& v){ point3DFromString(*this,v); }
    QString ToString() const { return point3DToString(*this); }
};
Q_DECLARE_TYPEINFO(Vector3F, Q_PRIMITIVE_TYPE);

typedef Vector3F Point3F;

#pragma pack()

class BoundingRect
{
    Point2F m_left;
    Point2F m_right;
public:
    BoundingRect() {}
    BoundingRect(const Point2F& left, const Point2F& right) : m_left(left), m_right(right) {}

    float Left() const { return m_left.x(); }
    float Top() const { return m_left.y(); }
    float Right() const { return m_right.x(); }
    float Bottom() const { return m_right.y(); }

    const Point2F& GetLeft() const { return m_left; }
    const Point2F& GetRight() const { return m_right; }
    float Width() const { return GetWidth(); }
    float Height() const { return GetHeight(); }
    float GetWidth() const { return m_right.X() - m_left.X(); }
    float GetHeight() const { return m_right.Y() - m_right.Y(); }

    bool Intersects(const BoundingRect& other) const
    {
        return !(m_right.X() < other.m_left.X() || m_left.X() > other.m_right.X() ||
                 m_right.Y() < other.m_left.Y() || m_left.Y() > other.m_right.Y()
                 );
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

    void SetNull()
    {
        m_topLeftFront = m_bottomRightBack;
    }

    void MultiplyExtent(float multiplier)
    {
        auto currentExtent = Extent();
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

    Point3F Extent() const { return Point3F(m_bottomRightBack.x() - m_topLeftFront.x(), m_topLeftFront.y() - m_bottomRightBack.y(), m_topLeftFront.z() - m_bottomRightBack.z()); }
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
        return !(m_bottomRightBack.x() < other.m_topLeftFront.x() || m_topLeftFront.x() > other.m_bottomRightBack.x() ||
                 m_bottomRightBack.y() > other.m_topLeftFront.y() || m_topLeftFront.y() < other.m_bottomRightBack.y() ||
                 m_bottomRightBack.z() > other.m_topLeftFront.z() || m_topLeftFront.z() < other.m_bottomRightBack.z()
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

#pragma pack(1)
struct TexturedVertex2F
{
    Point2F Position;
    Point2F TexCoord;
};

struct ColoredVertex2F
{
    Point2F Position;
    Color3F Color;
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

    bool operator==(const StatedVertex3F& another) const { return another.State == State && another.Position == Position; }
};

#pragma pack()


#define GT_CONTENT_PATH "../../../Content/"
#define GT_STYLES_PATH GT_CONTENT_PATH "CSS/"
#define GT_SHADERS_PATH ":/"

#endif // QT_GUI_LIB

#endif // GT_DECL_H
