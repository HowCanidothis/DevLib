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
        State_ConstantZ = 0x100,

        State_NeedUpdateProjection = State_ChangedProjection | State_ChangedWorld,
        State_NeedUpdateView = State_ChangedView | State_ChangedWorld,
        State_NeedUpdate = State_ChangedProjection | State_ChangedView | State_ChangedWorld,
        State_NeedAdjustScale = State_Isometric | State_AutoIsometricScaling
    };
    DECL_FLAGS(State,StateFlags)
    State m_state;

    SizeF m_viewport;
    Matrix4 m_projection;
    Matrix4 m_view;
    Matrix4 m_world;
    Matrix4 m_worldInverted;
    Matrix4 m_rotation;
    Matrix4 m_viewportProjection;

    Vector3F m_axis;
    Point3F m_eye;    
    Vector3F m_forward;
    Vector3F m_up;
    Point3F m_rotationPoint;

    void clone(GtCameraState* dst, GtCameraState* src) {
        *dst = *src;
    }
};

class GtCamera;

class GtCameraFocus
{
    Point3F m_scenePoint;
public:
    GtCameraFocus(GtCamera* target, const Point2I& screenPoint, float depth);
    GtCameraFocus(GtCamera* target, const Point3F& scenePoint);
    const Point3F& GetScenePoint() const { return m_scenePoint; }
};

class GtCamera : public GtCameraState
{
    friend class GtCameraStateSaver;
    class CameraObserverProperties;

    float m_near;
    float m_far;
    float m_angle;
    Point2F m_isometricScale;
    Point2F m_isometricExtraScale;
    Point2F m_isometricCoef;
    Point3F m_isometricCenter;
    float m_isometricCurtain;

    BoundingBox m_sceneBox;
    ScopedPointer<GtCameraFocus> m_focus;
    CameraObserverProperties* m_observer;
    std::function<void (qint32&, qint32&)> m_invertRotation;
    Vector3F m_projectionPlane;

public:
    GtCamera();
    ~GtCamera();

    void Normalize();
    void SetUp(const Vector3F& up);
    void SetForward(const Vector3F& forward);
    void SetEye(const Point3F& eye);
    void SetPosition(const Point3F& eye, const Point3F& center);
    void SetPosition(const Point3F& eye, const Vector3F& forward, const Vector3F& up);
    void MoveForward(float value);
    void MoveSide(float value);

    void MoveFocused(const Point2I& screenPosition);
    void MoveFocused(const Point2F& screenPosition);
    void Translate(float dx, float dy);
    void SetSceneBox(const BoundingBox& box) { this->m_sceneBox = box; }
    void FocusBind(const Point2I& screen_position, float depth);
    void FocusBind(const Point3F& worldPosition);
    void FocusRelease();
    void Zoom(bool closer);
    void Rotate(const Point2I& angles) { Rotate(angles.x() , angles.y()); }
    void Rotate(qint32 angleZ, qint32 angleX);
    void RotateRPE(const Point2I& angles) { RotateRPE(angles.x() , angles.y()); }
    void RotateRPE(qint32 angleZ, qint32 angleX);

    enum InvertRotationFlag {
        InvertNone,
        InvertLeftRight,
        InvertUpDown,
        InvertBoth
    };
    DECL_FLAGS(InvertRotationFlags, InvertRotationFlag);

    void InvertRotation(InvertRotationFlags invert);
    void SetAxisSystem(bool leftHanded);
    void SetIsometricCenterAndCertain(const Point3F& center, float distanceFromCenter);
    void SetIsometricScale(const Point2F& scale);
    void SetIsometricExtraScale(const Point2F& extraScale);
    void AddIsometricExtraScale(const Point2F& extraScale);
    void InitializeIsometric();
    void SetIsometric(bool flag);
    void Resize(qint32 width, qint32 height);
    void SetProjectionProperties(float m_angle, float near, float far);

    Point3F Unproject(float x, float y, float depth);
    Point3F Unproject(const Point2I& screen, float depth) { return Unproject(screen.x(), screen.y(), depth); }
    Point3F UnprojectPlane(float x, float y);
    Point3F UnprojectPlane(const Point2I& screen) { return UnprojectPlane(screen.x(),screen.y()); }

    Point3F Project(float x, float y, float z);
    Point3F Project(const Point3F& point);

    bool IsFrameChangedReset();
    bool IsFrameChanged() const;
    bool IsIsometric() const { return m_state.TestFlag(State_Isometric); }
    const Point2F& GetIsometricScale() const { return m_isometricScale; }
    const Point2F& GetIsometricExtraScale() const { return m_isometricExtraScale; }
    const Point3F& GetIsometricCenter() const { return m_isometricCenter; }
    float GetIsometricCurtain() const { return m_isometricCurtain; }

    const Point3F& GetEye() const { return m_eye; }
    const Vector3F& GetForward() const { return m_forward; }
    const Vector3F& GetUp() const { return m_up; }
    float GetFar() const { return m_far; }

    const Matrix4& GetProjection() { updateProjection(); return m_projection; }
    const Matrix4& GetView() { updateView(); return m_view; }
    const Matrix4& GetWorld() { updateWorld(); return m_world; }
    const Matrix4& GetWorldInverted() { updateWorld(); return m_worldInverted; }
    const Matrix4& GetRotation() { updateView(); return m_rotation; }
    const Matrix4& GetViewportProjection() { updateProjection(); return m_viewportProjection; }
    const SizeF& GetViewport() const { return m_viewport; }
    SizeF CalculateVisibleSize() { return visibleSize(); }

    void InstallObserver(const QString& path);

private:
    friend class GtCameraStateProperties;

    void updateWorld();
    void updateProjection();
    void updateView();

    void validateCameraPosition(Point3F &p) const;
    void adjustIsometricScale();
    void calculateIsometricCoef();

    SizeF predicateVisibleSizeOnZ(const SizeF& viewport, float z, bool ortho);
    SizeF visibleSize();

    Point3F unprojectFocused(const Point2F& screenPosition);
};

#endif // CAMERA_H
