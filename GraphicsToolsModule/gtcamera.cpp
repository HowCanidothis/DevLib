#include "gtcamera.h"
#include <PropertiesModule/internal.hpp>

class GtCamera::CameraObserverProperties
{
    ExternalVector3FProperty eye;
    ExternalVector3FProperty forward;
    ExternalVector3FProperty up;
public:
    CameraObserverProperties(const QString& path, GtCamera* camera)
        : eye(path+"/eye", camera->m_eye)
        , forward(path+"/forward", camera->m_forward)
        , up(path+"/up", camera->m_up)
    {
        auto assignOnSet = [camera](ExternalVector3FProperty& src, bool read_only){
            src.Subscribe([camera]{
                camera->m_state.AddFlag(GtCamera::State_NeedUpdateView);
            });

            src.SetReadOnly(read_only);
        };

        assignOnSet(eye, false);
        assignOnSet(forward, true);
        assignOnSet(up, true);
    }
};

GtCameraState::GtCameraState()
    : m_state(State_NeedUpdate)
    , m_axis(1.f, 1.f, 1.f)
    , m_eye(0.f,0.f,3.f)
    , m_forward(0.f,0.f,-1.f)
    , m_up(0.f,1.f,0.f)
{

}

class GtCameraStateSaver : protected GtCameraState
{
    GtCamera* camera;
public:
    GtCameraStateSaver(GtCamera* camera) : camera(camera) { clone(this, camera); camera->m_state.AddFlag(State_PredictionMode); }
    ~GtCameraStateSaver() { clone(camera, this); }
};

class GtCameraFocus
{
    Point3F m_scenePoint;
public:
    GtCameraFocus(GtCamera* target, const Point2I& screenPoint, float depth)
        : m_scenePoint(qFuzzyCompare((double)depth, 1.0) ? target->Unproject(screenPoint, 0.9999) : target->Unproject(screenPoint,depth))
    {}
    GtCameraFocus(GtCamera* target, const Point3F& scenePoint)
        : m_scenePoint(scenePoint)
    {}
    const Point3F& GetScenePoint() const { return m_scenePoint; }
};


GtCamera::GtCamera()
    : m_near(1.f)
    , m_far(150.f)
    , m_angle(45.f)
    , m_isometricCurtain(0.f)
    , m_focus(nullptr)
    , m_observer(nullptr)
    , m_invertRotation([](qint32&,qint32&){})
{

}

GtCamera::~GtCamera()
{

}

void GtCamera::Normalize()
{
    if(m_up != Vector3F(0.f,-1.f,0.f)){
        m_forward = Vector3F(0.f,0.f,-1.f);
        m_up = Vector3F(0.f,-1.f,0.f);
        m_state.AddFlag(State_NeedUpdateView);
    }
}

void GtCamera::SetForward(const Vector3F& forward)
{
    m_forward = forward;
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::SetEye(const Point3F& eye)
{
    m_eye = eye;
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::SetPosition(const Point3F& eye, const Point3F& center)
{
    this->m_eye = eye;
    this->m_forward = (center - eye).normalized();
    this->m_up = Vector3F::crossProduct(Vector3F(m_forward.y(), -m_forward.x(), 0.f), m_forward);
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::SetPosition(const Point3F& eye, const Vector3F& forward, const Vector3F& up)
{
    this->m_eye = eye;
    this->m_forward = forward;
    this->m_up = up;
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::MoveForward(float value)
{
    m_eye += m_forward * value;
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::MoveSide(float value)
{
    Vector3F side = Vector3F::crossProduct(-m_forward, m_up).normalized();
    m_eye += side * (value * m_axis.x());
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::MoveFocused(const Point2I& screenPosition)
{
    MoveFocused(Point2F(screenPosition.x(), screenPosition.y()));
}

void GtCamera::MoveFocused(const Point2F& screenPosition)
{
    if(m_focus != nullptr) {
        auto newPoint = unprojectFocused(screenPosition);
        const auto& oldPoint = m_focus->GetScenePoint();
        m_eye += (oldPoint - newPoint);
        m_state.AddFlag(State_NeedUpdateView);
    }
}

void GtCamera::Translate(float dx, float dy)
{
    m_eye.setX(m_eye.x() + dx);
    m_eye.setY(m_eye.y() + dy);
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::FocusRelease()
{
    m_focus = nullptr;
}

void GtCamera::FocusBind(const Point2I& screen_position, float depth)
{
    m_focus = new GtCameraFocus(this, screen_position, depth);
}

void GtCamera::FocusBind(const Point3F& worldPosition)
{
    m_focus = new GtCameraFocus(this, worldPosition);
}

void GtCamera::Zoom(bool closer)
{
    Vector3F ray;
    if(m_focus != nullptr) {
        ray = m_focus->GetScenePoint() - m_eye;
    } else {
        return;
    }

    if(closer && ray.lengthSquared() < m_near) {
        return;
    }

    if(m_state.TestFlag(State_Isometric)) {
        m_state.RemoveFlag(State_AutoIsometricScaling);
        auto screenPoint = Project(m_focus->GetScenePoint());

        m_isometricScale *= closer ? 0.75f : 1.25f;
        auto distance = m_isometricScale.x() * m_isometricCoef.x();
        m_eye = m_focus->GetScenePoint() - ray.normalized() * distance;

        m_state.AddFlags(State_NeedUpdateProjection | State_NeedUpdateView);
        MoveFocused(Point2F(screenPoint.x(), screenPoint.y()));
    } else {
        float denum = closer ? 4.f : -4.f;
        Point3F neye = m_eye + ray / denum;
        m_eye = neye;
        m_state.AddFlag(State_NeedUpdateView);
    }
}

void GtCamera::Rotate(qint32 angleZ, qint32 angleX)
{
    m_invertRotation(angleX, angleZ);

    Vector3F norm = Vector3F::crossProduct(m_up, m_forward).normalized();
    Quaternion rotZ = Quaternion::fromAxisAndAngle(m_up.normalized(),angleZ);
    Quaternion rot = Quaternion::fromAxisAndAngle(norm, -angleX);
    Quaternion rotation;
    if(m_state.TestFlag(State_NoRPE))
        rotation = rotZ;
    else
        rotation = rotZ * rot;

    if(m_focus != nullptr) {
        Matrix4 rm;
        const auto& rotationPoint = m_focus->GetScenePoint();
        rm.translate(rotationPoint.x(), rotationPoint.y(), rotationPoint.z());
        rm.rotate(rotation);
        rm.translate(-rotationPoint.x(), -rotationPoint.y(), -rotationPoint.z());

        Point3F neye = rm * m_eye;
        Vector3F nforward((rm * Vector4F(m_forward, 0.f)).toVector3D());
        Vector3F nup((rm * Vector4F(m_up, 0.f)).toVector3D());

    //if((neye.z() > m_sceneBox.Farthest()) && (nforward.z() < -0.02f) && nup.z() > 0.f){
        m_eye = neye;
        m_forward = nforward;
        m_up = nup;
    }
    /*}
    else{
        rm.setToIdentity();
        rm.translate(m_rotationPoint.x(), m_rotationPoint.y());
        rm.rotate(rotZ);
        rm.translate(-m_rotationPoint.x(), -m_rotationPoint.y());

        m_eye = rm * m_eye;
        m_forward = (rm * Vector4F(m_forward, 0.f)).toVector3D();
        m_up = (rm * Vector4F(m_up, 0.f)).toVector3D();
    }*/
    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::RotateRPE(qint32 angleZ, qint32 angleX)
{
    m_invertRotation(angleX, angleZ);

    Vector3F norm = Vector3F::crossProduct(m_up, m_forward).normalized();
    Quaternion rotZ = Quaternion::fromAxisAndAngle(m_up.normalized(),angleZ);
    Quaternion rot = Quaternion::fromAxisAndAngle(norm, -angleX);
    Quaternion rotation;
    if(m_state.TestFlag(State_NoRPE))
        rotation = rotZ;
    else
        rotation = rotZ * rot;

    Matrix4 rm;
    rm.rotate(rotation);

    Vector3F nforward((rm * Vector4F(m_forward, 0.f)).toVector3D());
    Vector3F nup((rm * Vector4F(m_up, 0.f)).toVector3D());

    m_forward = nforward;
    m_up = nup;

    m_state.AddFlag(State_NeedUpdateView);
}

void GtCamera::InvertRotation(InvertRotationFlags invert)
{
    switch(invert) {
    case InvertBoth: m_invertRotation = [](qint32& x, qint32&) { x = -x; }; break;
    case InvertUpDown: m_invertRotation = [](qint32& , qint32& z) { z = -z; }; break;
    case InvertLeftRight: m_invertRotation = [](qint32& x, qint32& ) { x = -x; }; break;
    default: m_invertRotation = [](qint32&,qint32&){}; break;
    };
}

void GtCamera::SetAxisSystem(bool leftHanded)
{
    if(leftHanded) {
        m_axis = { 1.f, 1.f, 1.f };
    } else {
        m_axis = { -1.f, 1.f, 1.f };
        InvertRotation(InvertUpDown);
    }
}

void GtCamera::SetIsometricScale(const Point2F& scale) {
    m_isometricScale = scale;
    m_state.RemoveFlag(State_AutoIsometricScaling);
    m_state.AddFlag(State_NeedUpdateProjection);
}

void GtCamera::SetIsometric(bool flag)
{
    m_state.AddFlag(State_NeedUpdate);
    m_state.ChangeFromBoolean(State_Isometric | State_AutoIsometricScaling, flag);
}

void GtCamera::Resize(qint32 width, qint32 height)
{
    m_viewport = SizeF(width,height);
    calculateIsometricCoef();
    m_state.AddFlag(State_NeedUpdateProjection);
}

void GtCamera::SetProjectionProperties(float angle, float _near, float _far)
{
    this->m_angle = angle;
    this->m_near = _near;
    this->m_far = _far;
    m_state.AddFlag(State_NeedUpdateProjection);
}

Point3F GtCamera::Unproject(float x, float y, float depth)
{
    const Matrix4& inverted = GetWorldInverted();
    Point4F coord(x / m_viewport.width() * 2.0 - 1.0,
                    (m_viewport.height() -  y) / m_viewport.height() * 2.0 - 1.0,
                    2.0 * depth - 1.0,
                    1.0
                    );
    coord = inverted * coord;
    if(!coord.w())
        return Point3F();
    coord.setW(1.0 / coord.w());

    return Point3F(coord.x() * coord.w(),
                     coord.y() * coord.w(),
                     coord.z() * coord.w());
}

Point3F GtCamera::UnprojectPlane(float x, float y)
{
    Point3F unproj0 = Unproject(x, y, 0.f);
    Point3F unproj1 = Unproject(x, y, 1.f);

    Vector3F rayDirection = unproj1 - unproj0;
    float dist = -unproj0.z() / rayDirection.z();
    return unproj0 + rayDirection * dist;
}

Point3F GtCamera::Project(float x, float y, float z)
{
    Vector4F tmp(x, y, z, 1.0f);
    tmp = GetWorld() * tmp;
    if(!tmp.w())
        return Point3F();
    tmp /= tmp.w();

    x = (tmp.x() * 0.5 + 0.5) * m_viewport.width();
    y = (tmp.y() * 0.5 + 0.5) * m_viewport.height();
    z = (1.0 + tmp.z()) * 0.5;

    return Point3F(x,m_viewport.height() - y,z);
}

Point3F GtCamera::Project(const Point3F& point)
{
    return Project(point.x(), point.y(), point.z());
}

bool GtCamera::IsFrameChangedReset()
{
    bool res = IsFrameChanged();
    m_state.RemoveFlag(State_FrameChanged);
    return res;
}

bool GtCamera::IsFrameChanged() const
{
    return m_state.TestFlagsAtLeastOne(State_FrameChanged | State_ChangedWorld);
}

void GtCamera::InstallObserver(const QString& path)
{
    Q_ASSERT(m_observer == nullptr);
    m_observer = new CameraObserverProperties(path, this);
}

void GtCamera::updateWorld()
{
    if(m_state.TestFlag(State_ChangedWorld)) {
        m_state.RemoveFlag(State_ChangedWorld);
        adjustIsometricScale();
        m_world = GetProjection() * GetView();
        m_worldInverted = m_world.inverted();
        m_state.AddFlag(State_FrameChanged);
    }
}

Point3F GtCamera::unprojectFocused(const Point2F& screenPosition)
{
    Q_ASSERT(m_focus != nullptr);
    auto x = screenPosition.x(), y = screenPosition.y();
    Point3F unproj0 = Unproject(x, y, 0.f);
    Point3F unproj1 = Unproject(x, y, 1.f);

    Vector3F rayDirection = unproj1 - unproj0;
    float dist = (m_focus->GetScenePoint() - unproj0).length();
    return unproj0 + rayDirection.normalized() * dist;
}

void GtCamera::SetIsometricCenterAndCertain(const Point3F& center, float distanceFromCenter)
{
    m_isometricCenter = center;
    m_isometricCurtain = distanceFromCenter;
}

void GtCamera::updateProjection()
{
    if(m_state.TestFlag(State_ChangedProjection)) {
        m_projection.setToIdentity();
        m_viewportProjection.setToIdentity();
        float w = m_viewport.width() / 2.f;
        float h = m_viewport.height() / 2.f;
        m_viewportProjection.ortho(-w, w, -h, h, m_near, m_far);
        if(m_state.TestFlag(State_Isometric)) {
            float w = m_viewport.width() * m_isometricScale.x();
            float h = m_viewport.height() * m_isometricScale.y();
            m_projection.ortho(-w, w,-h, h, -m_far, m_far);
        }
        else {
            m_projection.perspective(m_angle, float(m_viewport.width()) / m_viewport.height(), m_near, m_far);
        }
        m_projection.scale(m_axis);
        m_state.RemoveFlag(State_ChangedProjection);
    }
}

void GtCamera::updateView()
{
    if(m_state.TestFlag(State_ChangedView)) {
        m_view.setToIdentity();
        m_rotation.setToIdentity();

        Vector3F side = Vector3F::crossProduct(-m_forward, m_up).normalized();
        Vector3F upVector = Vector3F::crossProduct(side, -m_forward);

        m_view.setRow(0, Vector4F(-side, 0));
        m_view.setRow(1, Vector4F(upVector,0));
        m_view.setRow(2, Vector4F(-m_forward,0));
        m_view.setRow(3, Vector4F(0,0,0,1));
        m_view.translate(-m_eye);

        m_rotation.setRow(0, Vector4F(-side * m_axis.x(), 0));
        m_rotation.setRow(1, Vector4F(upVector,0));
        m_rotation.setRow(2, Vector4F(-m_forward,0));
        m_rotation.setRow(3, Vector4F(0,0,0,1));

        m_state.RemoveFlag(State_ChangedView);
    }
}

void GtCamera::adjustIsometricScale()
{
    if(m_state.TestFlagsAll(State_NeedAdjustScale) && !m_isometricCoef.isNull())
    {
        float distance;
        if(m_focus != nullptr) {
            distance = Vector3F::dotProduct(m_focus->GetScenePoint() - m_eye, m_forward) / m_forward.lengthSquared();
        } else {
            distance = m_isometricCurtain + Vector3F::dotProduct(m_isometricCenter - m_eye, m_forward);
        }

        Point2F isometricScale = Point2F(distance,distance) / m_isometricCoef;
        if(!qFuzzyCompare(isometricScale.x(), m_isometricScale.x()) || !qFuzzyCompare(isometricScale.y(), m_isometricScale.y())) {
            m_isometricScale = isometricScale;
            m_state.AddFlag(State_NeedUpdateProjection);
        }
    }
}

void GtCamera::calculateIsometricCoef()
{
    m_isometricScale = Point2F(1.f,1.f);
    m_isometricCoef = Point2F();
    auto size = predicateVisibleSizeOnZ(m_viewport, 10000.f, false);
    auto isometricSize = predicateVisibleSizeOnZ(m_viewport, 10000.f, true);
    m_isometricCoef.X() = 10000.f * isometricSize.width() / size.width();
    m_isometricCoef.Y() = 10000.f * isometricSize.height() / size.height();
}

SizeF GtCamera::predicateVisibleSizeOnZ(const SizeF& viewport, float z, bool ortho)
{
    GtCameraStateSaver c(this); Q_UNUSED(c)
    m_viewport = SizeF(viewport.width(), viewport.height());
    m_eye = Vector3F(m_eye.x(), m_eye.y(), z);
    m_forward = Vector3F(0.f,0.f,-1.f);
    m_up = Vector3F(0.f,1.f,0.f);

    m_state.AddFlag(State_NeedUpdate);
    m_state.ChangeFromBoolean(State_Isometric, ortho);
    return visibleSize();
}

SizeF GtCamera::visibleSize()
{
    qint32 w = m_viewport.width();
    qint32 h = m_viewport.height();
    Point3F v[4];
    v[0] = UnprojectPlane(0,0);
    v[1] = UnprojectPlane(0,h);
    v[2] = UnprojectPlane(w,h);

    return SizeF((v[2] - v[0]).length(), (v[1] - v[0]).length());
}
