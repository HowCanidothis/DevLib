#ifndef SHAREDMODULE_DECL_H
#define SHAREDMODULE_DECL_H

#include <SharedModule/internal.hpp>

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QDateTime>
#include <QOpenGLFunctions_4_5_Core>

typedef QOpenGLFunctions_4_5_Core OpenGLFunctions;
typedef QQuaternion Quaternion;
typedef QPoint Point2I;

typedef QVector4D Point4F;
typedef Point4F Vector4F;
typedef QMatrix4x4 Matrix4;
typedef QSizeF SizeF;
typedef QSize SizeI;

template<class T> inline static void point2DFromString(T& point, const QString& string){
    static QRegExp re("(\\()?([^,]+),([^,\\)]+)\\)?");
    re.indexIn(string);
    point.X() = re.cap(2).toDouble();
    point.Y() = re.cap(3).toDouble();
}
template<class T> inline static void point3DFromString(T& point, const QString& string){
    static QRegExp re("(\\()?([^,]+),([^,]+),([^,\\)]+)\\)?");
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
    using Super::Super;

    float& X() { return operator[](0); }
    float X() const { return operator[](0); }

    float& Y() { return operator[](1); }
    float Y() const { return operator[](1); }

    Vector2F ortho() const { return Vector2F(-y(), x()); }

    operator QVector2D&() { return *(QVector2D*)this; }
    operator const QVector2D&() const { return *(QVector2D*)this; }

    bool operator <(const Vector2F& other) const{
        if(!qFuzzyIsNull(x() - other.x())) return x() < other.x();
        return y() < other.y();
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

    float& X() { return operator[](0); }
    float X() const { return operator[](0); }

    float& Y() { return operator[](1); }
    float Y() const { return operator[](1); }

    float& Z() { return operator[](2); }
    float Z() const { return operator[](2); }

    operator QVector3D&() { return *(QVector3D*)this; }
    operator const QVector3D&() const { return *(QVector3D*)this; }

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
    Point2F _left;
    Point2F _right;
public:
    BoundingRect() {}
    BoundingRect(const Point2F& left, const Point2F& right) : _left(left), _right(right) {}

    const Point2F& GetLeft() const { return _left; }
    const Point2F& GetRight() const { return _right; }
    float Width() const { return GetWidth(); }
    float Height() const { return GetHeight(); }
    float GetWidth() const { return _right.x() - _left.x(); }
    float GetHeight() const { return _right.y() - _right.y(); }

    bool Intersects(const BoundingRect& other) const
    {
        return !(_right.x() < other._left.x() || _left.x() > other._right.x() ||
                 _right.y() < other._left.y() || _left.y() > other._right.y()
                 );
    }
};

class BoundingBox
{
    Point3F _left;
    Point3F _right;
public:
    BoundingBox() {}
    BoundingBox(const Point3F& left, const Point3F& right) :
        _left(left),
        _right(right)
    {}

    void SetNull()
    {
        _left = _right;
    }

    const Point3F& GetLeft() const { return _left; }
    const Point3F& GetRight() const { return _right; }
    float X() const { return _left.x(); }
    float Y() const { return _left.y(); }
    float Right() const { return _right.x(); }
    float Bottom() const { return _right.y(); }
    float Farthest() const { return _left.z(); }
    float Nearest() const { return _right.z(); }

    bool Intersects(const BoundingBox& other) const
    {
        return !(_right.x() < other._left.x() || _left.x() > other._right.x() ||
                 _right.y() < other._left.y() || _left.y() > other._right.y() ||
                 _right.z() < other._left.z() || _left.z() > other._right.z()
                 );
    }
    BoundingBox& Unite(const BoundingBox& other)
    {
        const auto& otl = other.GetLeft();
        const auto& obr = other.GetRight();
        if(otl.x() < _left.x()) {
            _left.setX(otl.x());
        }
        if(otl.y() < _left.y()) {
            _left.setY(otl.y());
        }
        if(otl.z() < _left.z()) {
            _left.setZ(otl.z());
        }

        if(obr.x() > _right.x()) {
            _right.setX(obr.x());
        }
        if(obr.y() > _right.y()) {
            _right.setY(obr.y());
        }
        if(obr.z() > _right.z()) {
            _right.setZ(obr.z());
        }
        return *this;
    }

    bool IsNull() const { return _right == _left; }
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

#define GT_CONTENT_PATH "../../Content/"
#define GT_STYLES_PATH GT_CONTENT_PATH "CSS/"
#define GT_SHADERS_PATH GT_CONTENT_PATH "Shaders/"

#endif // GT_DECL_H
