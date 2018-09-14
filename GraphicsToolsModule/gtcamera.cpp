#include "gtcamera.h"
#include <PropertiesModule/internal.hpp>

class GtCamera::CameraObserverProperties
{
    ExternalVector3FProperty eye;
    ExternalVector3FProperty forward;
    ExternalVector3FProperty up;
public:
    CameraObserverProperties(const QString& path, GtCamera* camera)
        : eye(Name(path+"/eye"), camera->eye)
        , forward(Name(path+"/forward"), camera->forward)
        , up(Name(path+"/up"), camera->up)
    {
        auto assignOnSet = [camera](ExternalVector3FProperty& src, bool read_only){
            src.Subscribe([camera]{
                camera->state.AddFlag(GtCamera::State_NeedUpdateView);
            });

            src.SetReadOnly(read_only);
        };

        assignOnSet(eye, false);
        assignOnSet(forward, true);
        assignOnSet(up, true);
    }
};

GtCameraState::GtCameraState() :
    state(State_NeedUpdate),
    eye(0.f,0.f,3.f),
    forward(0.f,0.f,-1.f),
    up(0.f,1.f,0.f)
{

}

class GtCameraStateSaver : protected GtCameraState
{
    GtCamera* camera;
public:
    GtCameraStateSaver(GtCamera* camera) : camera(camera) { clone(this, camera); camera->state.AddFlag(State_PredictionMode); }
    ~GtCameraStateSaver() { clone(camera, this); }
};

class GtCameraFocus
{
     Point2I screen_point;
     Point3F scene_point;
public:
     GtCameraFocus(GtCamera* target, const Point2I& screen_point) :
         screen_point(screen_point),
         scene_point(target->unprojectPlane(screen_point))
     {}
     void focus(GtCamera* target) {
         Vector3F now = target->unprojectPlane(screen_point);
         target->translate(scene_point.x() - now.x(), scene_point.y() - now.y());
     }
};


GtCamera::GtCamera()
    : nearc(1.f)
    , farc(150.f)
    , angle(45.f)
    , isometric_scale(1.f)
    , isometric_coef(0.f)
    , focus(nullptr)
    , observer(nullptr)
{

}

GtCamera::~GtCamera()
{

}

void GtCamera::normalize()
{
    if(up != Vector3F(0.f,-1.f,0.f)){
        forward = Vector3F(0.f,0.f,-1.f);
        up = Vector3F(0.f,-1.f,0.f);
        state.AddFlag(State_NeedUpdateView);
    }
}

void GtCamera::setPosition(const Point3F& eye, const Point3F& center)
{
    this->eye = eye;
    this->forward = (center - eye).normalized();
    this->up = Vector3F::crossProduct(Vector3F(forward.y(), -forward.x(), 0.f), forward);
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::setPosition(const Point3F& eye, const Point3F& forward, const Vector3F& up)
{
    this->eye = eye;
    this->forward = forward;
    this->up = up;
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::moveForward(float value)
{
    eye += forward * value;
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::moveSide(float value)
{
    Vector3F side = Vector3F::crossProduct(-forward, up).normalized();
    eye += side * value;
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::translate(float dx, float dy)
{
    eye.setX(eye.x() + dx);
    eye.setY(eye.y() + dy);
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::focusBind(const Point2I& screen_position)
{
    focus = new GtCameraFocus(this, screen_position);
}

void GtCamera::focusRelease()
{
    if(focus) {
        focus->focus(this);
        focus = nullptr;
    }
}

void GtCamera::zoom(bool closer)
{
    Vector3F ray(getCenter() - eye);
    float denum = closer ? 4.f : -4.f;
    Point3F neye = eye + ray / denum;
    eye = neye;

    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::rotate(qint32 angleZ, qint32 angleX)
{
    Vector3F norm = Vector3F::crossProduct(up, forward).normalized();
    Quaternion rotZ = Quaternion::fromAxisAndAngle(0,0,1,angleZ);
    Quaternion rot = Quaternion::fromAxisAndAngle(norm, angleX);
    Quaternion rotation;
    if(state.TestFlag(State_NoRPE))
        rotation = rotZ;
    else
        rotation = rotZ * rot;

    Matrix4 rm;
    rm.translate(rotation_point.x(), rotation_point.y());
    rm.rotate(rotation);
    rm.translate(-rotation_point.x(), -rotation_point.y());

    Point3F neye = rm * eye;
    Vector3F nforward = (rm * Vector4F(forward, 0.f)).toVector3D();
    Vector3F nup = (rm * Vector4F(up, 0.f)).toVector3D();

    if((neye.z() > scene_box.Farthest()) && (nforward.z() < -0.02f) && nup.z() > 0.f){
        eye = neye;
        forward = nforward;
        up = nup;
    }
    else{
        rm.setToIdentity();
        rm.translate(rotation_point.x(), rotation_point.y());
        rm.rotate(rotZ);
        rm.translate(-rotation_point.x(), -rotation_point.y());

        eye = rm * eye;
        forward = (rm * Vector4F(forward, 0.f)).toVector3D();
        up = (rm * Vector4F(up, 0.f)).toVector3D();
    }
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::rotateRPE(qint32 angleZ, qint32 angleX)
{
    Vector3F norm = Vector3F::crossProduct(up, forward).normalized();
    Quaternion rotZ = Quaternion::fromAxisAndAngle(0,0,1,angleZ);
    Quaternion rot = Quaternion::fromAxisAndAngle(norm, -angleX);
    Quaternion rotation;
    if(state.TestFlag(State_NoRPE))
        rotation = rotZ;
    else
        rotation = rotZ * rot;

    Matrix4 rm;
    rm.rotate(rotation);

    Vector3F nup = (rm * Vector4F(up, 0.0)).toVector3D();
    if(nup.z() >= 0){
        forward = (rm * Vector4F(forward, 0)).toVector3D();
        up = nup;
    }
    else{
        rm.setToIdentity();
        rm.rotate(rotZ);
        forward = (rm * Vector4F(forward, 0)).toVector3D();
        up = (rm * Vector4F(up, 0.0)).toVector3D();
    }
    state.AddFlag(State_NeedUpdateView);
}

void GtCamera::setIsometricScale(float scale) {
    isometric_scale = scale;
    state.RemoveFlag(State_AutoIsometricScaling);
    state.AddFlag(State_NeedUpdateProjection);
}

void GtCamera::setIsometric(bool flag) {
    state.AddFlag(State_NeedUpdate); state.ChangeFromBoolean(State_Isometric | State_AutoIsometricScaling, flag);
}


void GtCamera::resize(qint32 width, qint32 height)
{
    viewport = SizeF(width,height);
    calculateIsometricCoef();
    state.AddFlag(State_NeedUpdateProjection);
}

void GtCamera::setProjectionProperties(float angle, float _near, float _far)
{
    this->angle = angle;
    this->nearc = _near;
    this->farc = _far;
    state.AddFlag(State_NeedUpdateProjection);
}

Point3F GtCamera::unproject(float x, float y, float depth)
{
    const Matrix4& inverted = getWorldInverted();
    Point4F coord(x / viewport.width() * 2.0 - 1.0,
                    (viewport.height() -  y) / viewport.height() * 2.0 - 1.0,
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

Point3F GtCamera::unprojectPlane(float x, float y)
{
    Point3F unproj0 = unproject(x, y, 0.f);
    Point3F unproj1 = unproject(x, y, 1.f);

    Vector3F rayDirection = unproj1 - unproj0;
    float dist = -unproj0.z() / rayDirection.z();
    return unproj0 + rayDirection * dist;
}

Point3F GtCamera::project(float x, float y, float z)
{
    Vector4F tmp(x, y, z, 1.0f);
    tmp = getWorld() * tmp;
    if(!tmp.w())
        return Point3F();
    tmp /= tmp.w();

    x = (tmp.x() * 0.5 + 0.5) * viewport.width();
    y = (tmp.y() * 0.5 + 0.5) * viewport.height();
    z = (1.0 + tmp.z()) * 0.5;

    return Point3F(x,viewport.height() - y,z);
}

Point3F GtCamera::project(const Point3F& point)
{
    return project(point.x(), point.y(), point.z());
}

bool GtCamera::takeNewFrame()
{
    bool res = isFrameChanged();
    state.RemoveFlag(State_FrameChanged);
    return res;
}

bool GtCamera::isFrameChanged() const
{
    return state.TestFlagsAtLeastOne(State_FrameChanged | State_ChangedWorld);
}

void GtCamera::installObserver(const QString& path)
{
    Q_ASSERT(observer == nullptr);
    observer = new CameraObserverProperties(path, this);
}

void GtCamera::updateWorld()
{
    if(state.TestFlag(State_ChangedWorld)) {
        adjustIsometricScale();
        world = getProjection() * getView();
        world_inv = world.inverted();
        state.RemoveFlag(State_ChangedWorld);
        state.AddFlag(State_FrameChanged);
    }
}

void GtCamera::updateProjection()
{
    if(state.TestFlag(State_ChangedProjection)) {
        projection.setToIdentity();
        if(state.TestFlag(State_Isometric)) {
            float vws = viewport.width() * isometric_scale;
            float hws = viewport.height() * isometric_scale;
            projection.ortho(-vws, vws,-hws, hws, nearc, farc);
        }
        else {
            projection.perspective(angle, float(viewport.width()) / viewport.height(), nearc, farc);
        }
        state.RemoveFlag(State_ChangedProjection);
    }
}

void GtCamera::updateView()
{
    if(state.TestFlag(State_ChangedView)) {
        if(!state.TestFlag(State_PredictionMode)) validateCameraPostition(eye);

        view.setToIdentity();

        Vector3F side = Vector3F::crossProduct(-forward, up).normalized();
        Vector3F upVector = Vector3F::crossProduct(side, -forward);

        view.setRow(0, Vector4F(-side, 0));
        view.setRow(1, Vector4F(upVector,0));
        view.setRow(2, Vector4F(-forward,0));
        view.setRow(3, Vector4F(0,0,0,1));
        view.translate(-eye);

        state.RemoveFlag(State_ChangedView);
    }
}

void GtCamera::validateCameraPostition(Point3F& p) const
{
    if(scene_box.IsNull()) {
        return;
    }
    if(p.x() < scene_box.X()) p.setX(scene_box.X());
    else if(p.x() > scene_box.Right()) p.setX(scene_box.Right());
    if(p.y() < scene_box.Y()) p.setY(scene_box.Y());
    else if(p.y() > scene_box.Bottom()) p.setY(scene_box.Bottom());
    if(p.z() < scene_box.Farthest()) p.setZ(scene_box.Farthest());
    else if(p.z() > scene_box.Nearest()) p.setZ(scene_box.Nearest());
}

void GtCamera::adjustIsometricScale()
{
    if(state.TestFlagsAll(State_NeedAdjustScale) && isometric_coef != 0.f)
    {
        isometric_scale = eye.z() / isometric_coef;
        state.AddFlag(State_NeedUpdateProjection);
    }
}

void GtCamera::calculateIsometricCoef()
{
    isometric_scale = 1.f;
    BoundingRect rect = this->predicateVisibleRectOnZ(this->viewport, 10000.f, false);
    BoundingRect isometric_rect = this->predicateVisibleRectOnZ(this->viewport, 10000.f, true);
    isometric_coef = 10000.f * isometric_rect.Width() / rect.Width();
}

BoundingRect GtCamera::predicateVisibleRectOnZ(const SizeF& _viewport, float z, bool ortho)
{
    GtCameraStateSaver c(this); Q_UNUSED(c)
            this->viewport = SizeF(_viewport.width(), _viewport.height());
    this->eye = Vector3F(eye.x(), eye.y(), z);
    this->forward = Vector3F(0.f,0.f,-1.f);
    this->up = Vector3F(0.f,1.f,0.f);

    this->state.AddFlag(State_NeedUpdate);
    this->state.ChangeFromBoolean(State_Isometric, ortho);
    return getVisibleRect();
}

BoundingRect GtCamera::getVisibleRect()
{
    qint32 w = viewport.width();
    qint32 h = viewport.height();
    Point3F v[4];
    v[0] = unprojectPlane(0,0);
    v[1] = unprojectPlane(0,h);
    v[2] = unprojectPlane(w,h);
    v[3] = unprojectPlane(w,0);

    std::pair<Point3F*, Point3F*> mmx = std::minmax_element(v, v + 4, [](const Point3F& f, const Point3F& s){ return f.x() < s.x(); });
    std::pair<Point3F*, Point3F*> mmy =std::minmax_element(v, v + 4, [](const Point3F& f, const Point3F& s){ return f.y() < s.y(); });

    return BoundingRect(Point2F(mmx.first->x(), mmy.first->y()), Point2F(mmx.second->x(), mmy.second->y()));
}
