#ifndef GT_DECL_H
#define GT_DECL_H

#include "Shared/shared_decl.h"

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
    Point2F left;
    Point2F right;
public:
    BoundingRect() {}
    BoundingRect(const Point2F& left, const Point2F& right) : left(left), right(right) {}

    const Point2F& getLeft() const { return left; }
    const Point2F& getRight() const { return right; }
    float width() const { return getWidth(); }
    float height() const { return getHeight(); }
    float getWidth() const { return right.x() - left.x(); }
    float getHeight() const { return right.y() - right.y(); }

    bool intersects(const BoundingRect& other) const {
        return !(right.x() < other.left.x() || left.x() > other.right.x() ||
                 right.y() < other.left.y() || left.y() > other.right.y()
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

    const Point3F& getLeft() const { return _left; }
    const Point3F& getRight() const { return _right; }
    float x() const { return _left.x(); }
    float y() const { return _left.y(); }
    float right() const { return _right.x(); }
    float bottom() const { return _right.y(); }
    float farthest() const { return _left.z(); }
    float nearest() const { return _right.z(); }

    bool intersects(const BoundingBox& other) const {
        return !(_right.x() < other._left.x() || _left.x() > other._right.x() ||
                 _right.y() < other._left.y() || _left.y() > other._right.y() ||
                 _right.z() < other._left.z() || _left.z() > other._right.z()
                 );
    }

    bool isNull() const { return _left.isNull() && _right.isNull(); }
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
