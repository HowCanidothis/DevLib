#ifndef SHAREDMODULE_DECL_H
#define SHAREDMODULE_DECL_H

#include "SharedModule/decl.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QOpenGLFunctions_4_5_Core>

typedef QOpenGLFunctions_4_5_Core OpenGLFunctions;
typedef QQuaternion Quaternion;
typedef QPoint Point2I;

typedef QVector3D Point3F;
typedef Point3F Vector3F;
typedef Vector3F Color3F;
typedef QVector4D Point4F;
typedef Point4F Vector4F;
typedef QMatrix4x4 Matrix4;
typedef QSizeF SizeF;
typedef QSize SizeI;

class Vector2F : public QVector2D
{
public:
    Vector2F(const QVector2D& vector) : QVector2D(vector) {}
    Vector2F() : QVector2D() {}
    Vector2F(float x, float y) : QVector2D(x,y) {}

    Vector2F ortho() const { return Vector2F(-y(), x()); }

    operator QVector2D&() { return *(QVector2D*)this; }
    operator const QVector2D&() const { return *(QVector2D*)this; }
};

typedef Vector2F Point2F;

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
