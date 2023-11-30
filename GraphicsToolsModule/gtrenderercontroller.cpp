#include "gtrenderercontroller.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QOpenGLFramebufferObject>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>

#include <ControllersModule/internal.hpp>

#include "gtframebufferobject.h"
#include "gtdepthbuffer.h"
#include "gtcamera.h"
#include "decl.h"
#include "gtrenderer.h"
#include "gtrenderpath.h"

GtCameraAnimationEngine::GtCameraAnimationEngine(GtRenderer* renderer, GtCamera* camera)
    : m_camera(camera)
    , m_renderer(renderer)
    , m_rotationDuration(1000)
    , m_movementDuration(1000)
    , m_rotationCurve(QEasingCurve::InOutCubic)
    , m_movementCurve(QEasingCurve::OutCubic)
{
    m_renderer->OnAboutToBeDestroyed.Connect(CONNECTION_DEBUG_LOCATION,[this]{
        m_animation = nullptr;
        m_renderer = nullptr;
    }).MakeSafe(m_connections);
}

void GtCameraAnimationEngine::Move(const Point3F& center, const Vector3F& forward, const Vector3F& up, float distance){
    if(m_renderer == nullptr) {
        return;
    }
    auto normalizedUp = up.normalized();
    auto normalizedForward = forward.normalized();
    {
        QMutexLocker locker(&m_mutex);
        m_animation = new QParallelAnimationGroup();
    }
    auto eyeAnimation = new QVariantAnimation(m_animation.get());
    auto forwardAnimation = new QVariantAnimation(m_animation.get());
    auto upAnimation = new QVariantAnimation(m_animation.get());
    eyeAnimation->setStartValue(m_camera->GetEye());
    eyeAnimation->setEasingCurve(m_movementCurve);
    eyeAnimation->setDuration(m_movementDuration);
    eyeAnimation->setEndValue(center - normalizedForward * distance);

    QObject::connect(eyeAnimation, &QVariantAnimation::valueChanged, [this](const QVariant& value){
        m_renderer->Asynch([this, value]{
            m_camera->SetEye(value.value<QVector3D>());
        });
    });

    forwardAnimation->setStartValue(m_camera->GetForward());
    forwardAnimation->setEasingCurve(m_rotationCurve);
    forwardAnimation->setDuration(m_rotationDuration);
    forwardAnimation->setEndValue(normalizedForward);

    QObject::connect(forwardAnimation, &QVariantAnimation::valueChanged, [this](const QVariant& value){
        m_renderer->Asynch([this, value]{
            m_camera->SetForward(value.value<QVector3D>());
        });
    });

    upAnimation->setStartValue(m_camera->GetUp());
    upAnimation->setEasingCurve(m_rotationCurve);
    upAnimation->setDuration(m_rotationDuration);
    upAnimation->setEndValue(normalizedUp);

    QObject::connect(upAnimation, &QVariantAnimation::valueChanged, [this](const QVariant& value){
        m_renderer->Asynch([this, value]{
            m_camera->SetUp(value.value<QVector3D>());
        });
    });

    m_animation->addAnimation(eyeAnimation);
    m_animation->addAnimation(forwardAnimation);
    m_animation->addAnimation(upAnimation);

    m_animation->start();
}

void GtCameraAnimationEngine::Move(const Point3F& center, float distance)
{
    Move(center, m_camera->GetForward(), m_camera->GetUp(), distance);
}

void GtCameraAnimationEngine::Move(const Point3F& newEye)
{
    m_animation = new QParallelAnimationGroup();
    auto eyeAnimation = new QVariantAnimation(m_animation.get());
    eyeAnimation->setStartValue(m_camera->GetEye());
    eyeAnimation->setEasingCurve(m_movementCurve);
    eyeAnimation->setDuration(m_movementDuration);
    eyeAnimation->setEndValue(newEye);

    QObject::connect(eyeAnimation, &QVariantAnimation::valueChanged, [this](const QVariant& value){
        m_renderer->Asynch([this, value]{
            m_camera->SetEye(value.value<QVector3D>());
        });
    });

    m_animation->addAnimation(eyeAnimation);

    m_animation->start();
}

bool GtCameraAnimationEngine::IsRunning() const
{
    QMutexLocker locker(&m_mutex);
    if(m_animation == nullptr) {
        return false;
    }

    return m_animation->state() == QAbstractAnimation::Running;
}

GtRendererController::GtRendererController(GtRenderer* renderer, ControllersContainer* controllersContainer, GtControllersContext* context)
    : SpaceColor("1e1e1e")
    , Enabled(true)
    , m_renderer(renderer)
    , m_camera(new GtCamera())
    , m_controllersContext(context)
    , m_controllers(controllersContainer)
    , m_renderTime(0)    
    , m_cameraAnimationEngine(renderer, m_camera.get())
    , m_resize(DelayedCallObjectParams(100, renderer->GetThreadHandlerNoCheck()))
    , m_dirty(true)
    , m_renderPath(::make_shared<GtDefaultRenderPath>(renderer))
{
    m_controllersContext->Camera = m_camera.get();
    m_controllersContext->Renderer = renderer;
    m_controllers->SetContext(m_controllersContext.get());

    m_camera->SetProjectionProperties(45.f, 1.0f, 100000.f);
    m_camera->SetPosition({0.f,0.f,1000.f}, { 0.f, 0.f, -1.f }, { 0.f, 1.f, 0.f });

    m_renderer->MoveToThread(SpaceColor);
}

bool GtRendererController::isDirtyReset()
{
    auto result = m_dirty | GetCamera()->IsFrameChangedReset();
    m_dirty = false;
    return result;
}

void GtRendererController::drawSpace(OpenGLFunctions* f)
{
    const auto& color = SpaceColor.Native();
    f->glClearColor(color.redF(), color.greenF(), color.blueF(), 1.f);
    calculateVisibleSize();
}

void GtRendererController::UpdateFrame()
{
    m_dirty = true;
}

GtRendererController::~GtRendererController()
{
    OnAboutToBeDestroyed();
}

void GtRendererController::RemoveDrawable(qint32 queueNumber, GtDrawableBase* drawable)
{
    m_renderer->Asynch([this, queueNumber, drawable]{
        auto foundIt = m_drawables.find(queueNumber);
        if(foundIt != m_drawables.end()) {
            foundIt.value().removeOne(drawable);
            drawable->Destroy();
        }
    });
}

void GtRendererController::ClearQueue(qint32 queueNumber)
{
    m_renderer->Asynch([this, queueNumber]{
        auto foundIt = m_drawables.find(queueNumber);
        if(foundIt != m_drawables.end()) {
            for(auto* drawable : ::make_const(foundIt.value())) {
                drawable->Destroy();
            }
            m_drawables.erase(foundIt);
        }
    });
}

void GtRendererController::calculateVisibleSize()
{
    m_visibleSize = m_camera->CalculateVisibleSize();
}

void GtRendererController::SetRenderProperties(const GtRenderProperties& renderProperties)
{
    m_renderer->Asynch([this, renderProperties]{
        m_renderProperties = renderProperties;
    });
}

void GtRendererController::SetRenderProperty(const Name& name, const QVariant& value)
{
    m_renderer->Asynch([this, name, value]{
        m_renderProperties.insert(name, value);
    });
}

void GtRendererController::SetProjectionProperties(float angle, float nearValue, float farValue)
{
    m_renderer->Asynch([this, angle, nearValue, farValue]{
        m_camera->SetProjectionProperties(angle,nearValue,farValue);
    });
}

void GtRendererController::MouseMoveEvent(QMouseEvent* event)
{
    auto cevent = new QMouseEvent(*event);
    m_renderer->Asynch([this, cevent]{
        m_dirty = true;
        m_controllers->MouseMoveEvent(cevent);
        delete cevent;
    });
}

void GtRendererController::MousePressEvent(QMouseEvent* event)
{
    auto cevent = new QMouseEvent(*event);
    m_renderer->Asynch([this, cevent]{
        m_controllers->MousePressEvent(cevent);
        delete cevent;
    });
}

void GtRendererController::MouseReleaseEvent(QMouseEvent* event)
{
    auto cevent = new QMouseEvent(*event);
    m_renderer->Asynch([this, cevent]{
        m_controllers->MouseReleaseEvent(cevent);
        delete cevent;
    });
}

void GtRendererController::WheelEvent(QWheelEvent* event)
{
    auto cevent = new QWheelEvent(*event);
    m_renderer->Asynch([this, cevent]{
        m_controllers->WheelEvent(cevent);
        delete cevent;
    });
}

void GtRendererController::KeyPressEvent(QKeyEvent* event)
{
    auto cevent = new QKeyEvent(*event);
    m_renderer->Asynch([this, cevent]{
        m_controllers->KeyPressEvent(cevent);
        delete cevent;
    });
}

void GtRendererController::KeyReleaseEvent(QKeyEvent* event)
{
    auto cevent = new QKeyEvent(*event);
    m_renderer->Asynch([this, cevent]{
        m_controllers->KeyReleaseEvent(cevent);
        delete cevent;
    });
}

QImage GtRendererController::GetCurrentImage() const
{
    QMutexLocker locker(&m_outputImageMutex);
    if(m_outputImage != nullptr) {
        return *m_outputImage;
    }
    return QImage();
}

void GtRendererController::setCurrentImage(QImage* image, double renderTime)
{
    QMutexLocker locker(&m_outputImageMutex);
    m_outputImage = image;
    m_renderTime = renderTime;
    imageUpdated();
}

void GtRendererController::draw(OpenGLFunctions* f)
{
    for(const auto& queue : m_drawables) {
        for(auto* drawable : queue) {
            drawable->draw(f);
        }
    }
}

void GtRendererController::drawDepth(OpenGLFunctions* f)
{
    for(const auto& queue : m_drawables) {
        for(auto* drawable : queue) {
            drawable->drawDepth(f);
        }
    }
}

void GtRendererController::SetRenderPath(const GtRenderPathPtr& renderPath)
{
    m_renderPath = renderPath;
}

void GtRendererController::onInitialize()
{
    m_controllersContext->DepthBuffer = new GtDepthBuffer(m_renderer);
    m_renderPath->Initialize();
}

void GtRendererController::onDestroy()
{
    m_connections.clear();
    m_renderPath = nullptr;
    m_depthFbo = nullptr;
    m_fbo = nullptr;
    m_renderer = nullptr;
}

void GtRendererController::Resize(qint32 w, qint32 h)
{
    m_resize.Call(CONNECTION_DEBUG_LOCATION, [this, w, h]{
        if(m_renderer == nullptr) {
            return;
        }
        m_renderPath->Resize(w, h, m_renderer->m_surfaceFormat.samples());
        GtFramebufferFormat depthFboFormat;
        depthFboFormat.SetDepthAttachment(GtFramebufferFormat::Texture);

        auto depthFbo = new GtFramebufferObject(m_renderer, {w,h});
        depthFbo->Create(depthFboFormat);
        m_depthFbo = depthFbo;

        QOpenGLFramebufferObjectFormat format;
        format.setSamples(m_renderer->m_surfaceFormat.samples());
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo = new QOpenGLFramebufferObject(w, h, format);

        m_controllersContext->DepthBuffer->SetFrameBuffer(depthFbo, m_renderer->getContext());
        m_controllersContext->FrameBuffer = m_fbo.get();

        m_camera->Resize(w,h);
    });
}
