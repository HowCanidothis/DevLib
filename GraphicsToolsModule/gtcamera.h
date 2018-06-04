#ifndef GTCAMERA_H
#define GTCAMERA_H

#include "SharedModule/flags.h"
#include "SharedGuiModule/decl.h"

#include "SharedModule/external/external.hpp"

class GtCameraState
{
protected:
    GtCameraState();
    enum StateFlags{
        State_ChangedWorld = 0x1,
        State_ChangedProjection = 0x2,
        State_ChangedView = 0x4,
        State_Isometric = 0x8,
        State_AutoIsometricScaling = 0x10,
        State_NoRPE = 0x20,
        State_PredictionMode = 0x40,
        State_FrameChanged = 0x80,

        State_NeedUpdateProjection = State_ChangedProjection | State_ChangedWorld,
        State_NeedUpdateView = State_ChangedView | State_ChangedWorld,
        State_NeedUpdate = State_ChangedProjection | State_ChangedView | State_ChangedWorld,
        State_NeedAdjustScale = State_Isometric | State_AutoIsometricScaling
    };
    DECL_FLAGS(State,StateFlags)
    State state;

    SizeF viewport;
    Matrix4 projection;
    Matrix4 view;
    Matrix4 world;
    Matrix4 world_inv;

    Point3F eye;
    Vector3F forward;
    Vector3F up;
    Point3F rotation_point;

    void clone(GtCameraState* dst, GtCameraState* src) {
        *dst = *src;
    }
};

class GtCameraFocus;
class GtCamera;

class GtCamera : public GtCameraState
{
    friend class GtCameraStateSaver;
    class CameraObserverProperties;

    float nearc;
    float farc;
    float angle;
    float isometric_scale;
    float isometric_coef;

    BoundingBox scene_box;
    ScopedPointer<GtCameraFocus> focus;
    CameraObserverProperties* observer;
public:
    GtCamera();
    ~GtCamera();

    void normalize();
    void setPosition(const Point3F& eye, const Point3F& center);
    void setPosition(const Point3F& eye, const Point3F& forward, const Vector3F& up);
    void moveForward(float value);
    void moveSide(float value);

    void translate(float dx, float dy);
    void setSceneBox(const BoundingBox& box) { this->scene_box = box; }
    void focusBind(const Point2I& screen_position);
    void focusRelease();
    void setRotationPoint(const Point3F& position) { rotation_point = position; }
    void zoom(bool closer);
    void rotate(const Point2I& angles) { rotate(angles.x() , angles.y()); }
    void rotate(qint32 angleZ, qint32 angleX);
    void rotateRPE(const Point2I& angles) { rotateRPE(angles.x() , angles.y()); }
    void rotateRPE(qint32 angleZ, qint32 angleX);

    void setIsometricScale(float scale);
    void setIsometric(bool flag);
    void resize(qint32 width, qint32 height);
    void setProjectionProperties(float angle, float near, float far);

    Point3F unproject(float x, float y, float depth);
    Point3F unproject(const Point2I& screen, float depth) { return unproject(screen.x(), screen.y(), depth); }
    Point3F unprojectPlane(float x, float y);
    Point3F unprojectPlane(const Point2I& screen) { return unprojectPlane(screen.x(),screen.y()); }

    Point3F project(float x, float y, float z);
    Point3F project(const Point3F& point);

    bool takeNewFrame();
    bool isFrameChanged() const;

    const Point3F& getEye() const { return eye; }
    const Vector3F& getForward() const { return forward; }
    const Vector3F& getUp() const { return up; }
    const Point3F getCenter() const {
        Point3F res = eye - forward * eye.z() / forward.z();
        return res;
    }

    const Matrix4& getProjection() { updateProjection(); return projection; }
    const Matrix4& getView() { updateView(); return view; }
    const Matrix4& getWorld() { updateWorld(); return world; }
    const Matrix4& getWorldInverted() { updateWorld(); return world_inv; }

    void installObserver(const QString& path);

private:
    friend class GtCameraStateProperties;

    void updateWorld();
    void updateProjection();
    void updateView();

    void validateCameraPostition(Point3F &p) const;
    void adjustIsometricScale();
    void calculateIsometricCoef();

    BoundingRect predicateVisibleRectOnZ(const SizeF &_viewport, float z, bool ortho);
    BoundingRect getVisibleRect();
};

#endif // CAMERA_H
