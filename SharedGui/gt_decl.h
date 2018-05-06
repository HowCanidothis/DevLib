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
    Point2F p_left;
    Point2F p_right;
public:
    BoundingRect() {}
    BoundingRect(const Point2F& left, const Point2F& right) : p_left(left), p_right(right) {}

    const Point2F& getLeft() const { return p_left; }
    const Point2F& getRight() const { return p_right; }
    float width() const { return getWidth(); }
    float height() const { return getHeight(); }
    float getWidth() const { return p_right.x() - p_left.x(); }
    float getHeight() const { return p_right.y() - p_right.y(); }

    bool intersects(const BoundingRect& other) const {
        return !(p_right.x() < other.p_left.x() || p_left.x() > other.p_right.x() ||
                 p_right.y() < other.p_left.y() || p_left.y() > other.p_right.y()
                 );
    }
};

class BoundingBox
{
    Point3F p_left;
    Point3F p_right;
public:
    BoundingBox() {}
    BoundingBox(const Point3F& left, const Point3F& right) :
        p_left(left),
        p_right(right)
    {}

    void setNull() { p_left = p_right; }

    const Point3F& getLeft() const { return p_left; }
    const Point3F& getRight() const { return p_right; }
    float x() const { return p_left.x(); }
    float y() const { return p_left.y(); }
    float right() const { return p_right.x(); }
    float bottom() const { return p_right.y(); }
    float farthest() const { return p_left.z(); }
    float nearest() const { return p_right.z(); }

    bool intersects(const BoundingBox& other) const {
        return !(p_right.x() < other.p_left.x() || p_left.x() > other.p_right.x() ||
                 p_right.y() < other.p_left.y() || p_left.y() > other.p_right.y() ||
                 p_right.z() < other.p_left.z() || p_left.z() > other.p_right.z()
                 );
    }
    BoundingBox& unite(const BoundingBox& other) {
        const auto& otl = other.getLeft();
        const auto& obr = other.getRight();
        if(otl.x() < p_left.x()) {
            p_left.setX(otl.x());
        }
        if(otl.y() < p_left.y()) {
            p_left.setY(otl.y());
        }
        if(otl.z() < p_left.z()) {
            p_left.setZ(otl.z());
        }

        if(obr.x() > p_right.x()) {
            p_right.setX(obr.x());
        }
        if(obr.y() > p_right.y()) {
            p_right.setY(obr.y());
        }
        if(obr.z() > p_right.z()) {
            p_right.setZ(obr.z());
        }
        return *this;
    }

    bool isNull() const { return p_right == p_left; }
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
