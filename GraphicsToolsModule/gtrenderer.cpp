#include "gtrenderer.h"

#include <QOpenGLDebugLogger>
#include <QOpenGLFramebufferObject>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>

#include <GraphicsToolsModule/internal.hpp>
#include <GraphicsToolsModule/gtdepthbuffer.h>
#include <GraphicsToolsModule/gtplayercontrollercamera.h>

GtRenderer::GtRenderer(const PropertiesScopeName& scopeName)
    : m_camera(new GtCamera())
{
    PropertiesSystem::Begin(this, scopeName);
    m_camera->InstallObserver("Camera");
    PropertiesSystem::End();
}

GtRenderer::~GtRenderer()
{
    Quit();
}

void GtRenderer::SetControllers(ControllersContainer* controllers)
{
    m_controllers = controllers;
}

void GtRenderer::AddDrawable(GtDrawableBase* drawable)
{
    Asynch([this, drawable]{
        drawable->initialize(this);
        m_scene->AddDrawable(drawable);
    });
}

void GtRenderer::Update(const std::function<void (OpenGLFunctions*)>& handler)
{
    Asynch([this, handler]{
        handler(this);
    });
}

void GtRenderer::MouseMoveEvent(QMouseEvent* event)
{
    auto cevent = new QMouseEvent(*event);
    Asynch([this, cevent]{
        m_controllers->MouseMoveEvent(cevent);
        delete cevent;
    });
}

void GtRenderer::MousePressEvent(QMouseEvent* event)
{
    auto cevent = new QMouseEvent(*event);
    Asynch([this, cevent]{
        m_controllers->MousePressEvent(cevent);
        delete cevent;
    });
}

void GtRenderer::MouseReleaseEvent(QMouseEvent* event)
{
    auto cevent = new QMouseEvent(*event);
    Asynch([this, cevent]{
        m_controllers->MouseReleaseEvent(cevent);
        delete cevent;
    });
}

void GtRenderer::WheelEvent(QWheelEvent* event)
{
    auto cevent = new QWheelEvent(*event);
    Asynch([this, cevent]{
        m_controllers->WheelEvent(cevent);
        delete cevent;
    });
}

void GtRenderer::KeyPressEvent(QKeyEvent* event)
{
    auto cevent = new QKeyEvent(*event);
    Asynch([this, cevent]{
        m_controllers->KeyPressEvent(cevent);
        delete cevent;
    });
}

void GtRenderer::KeyReleaseEvent(QKeyEvent* event)
{
    auto cevent = new QKeyEvent(*event);
    Asynch([this, cevent]{
        m_controllers->KeyReleaseEvent(cevent);
        delete cevent;
    });
}

QImage GtRenderer::CurrentImage()
{
    QMutexLocker locker(&m_outputImageMutex);
    if(m_outputImage != nullptr) {
        return *m_outputImage;
    }
    return QImage();
}


void GtRenderer::onInitialize()
{
    if(!initializeOpenGLFunctions()) {
        qCInfo(LC_SYSTEM) << "Cannot initialize opengl functions";
        return;
    }

    ResourcesSystem::RegisterResource("mvp", []{
        return new Matrix4();
    });

    ResourcesSystem::RegisterResource("invertedMVP", []{
        return new Matrix4();
    });

    ResourcesSystem::RegisterResource("eye", []{
        return new Vector3F();
    });

    ResourcesSystem::RegisterResource("forward", []{
        return new Vector3F();
    });

    if(m_controllers == nullptr) {
        m_controllers = new ControllersContainer();
        new GtPlayerControllerCamera(Name("GtPlayerControllerCamera"), m_controllers.get());
    }
    m_scene = new GtScene();

    m_camera->SetProjectionProperties(45.f, 1.0f, 100000.f);


    m_controllersContext = new GtControllersContext();
    m_controllersContext->Camera = m_camera.data();
    m_controllersContext->DepthBuffer = new GtDepthBuffer(this);

    m_controllers->SetContext(m_controllersContext.get());

    /*if(m_params->DebugMode) {
        auto* logger = new QOpenGLDebugLogger(this);
        if(logger->initialize()) {
            logger->startLogging();
        }
    }*/

    m_mvp = ResourcesSystem::GetResource<Matrix4>("mvp");
    m_eye = ResourcesSystem::GetResource<Vector3F>("eye");
    m_invertedMv = ResourcesSystem::GetResource<Matrix4>("invertedMVP");
    m_forward = ResourcesSystem::GetResource<Vector3F>("forward");

    // TODO. Must have state machine feather
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);

    glDepthFunc(GL_LEQUAL);
    glLineWidth(5.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.7f, 0.7f, 1.f);
}

void GtRenderer::onResize(qint32 w, qint32 h)
{
    glViewport(0,0, w, h);

    GtFramebufferFormat depthFboFormat;
    depthFboFormat.SetDepthAttachment(GtFramebufferFormat::Texture);

    auto depthFbo = new GtFramebufferObject(this, {w,h});
    depthFbo->Create(depthFboFormat);
    m_depthFbo = depthFbo;


    QOpenGLFramebufferObjectFormat format;
    format.setSamples(8);
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    m_fbo = new QOpenGLFramebufferObject(w, h, format);

    m_controllersContext->DepthBuffer->SetFrameBuffer(depthFbo, m_context.get());

    m_camera->Resize(w,h);
}

void GtRenderer::onDraw()
{
    if(!isInitialized()) {
        return;
    }

    m_mvp->Data().Set(m_camera->GetWorld());
    m_eye->Data().Set(m_camera->GetEye());
    m_forward->Data().Set(m_camera->GetForward());
    m_invertedMv->Data().Set(m_camera->GetView().inverted().transposed());

    m_fbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene->draw(this);

    m_fbo->release();

    m_depthFbo->Bind();

    glClear(GL_DEPTH_BUFFER_BIT);

    m_scene->draw(this);

    m_depthFbo->Release();

    {
        QMutexLocker locker(&m_outputImageMutex);
        m_outputImage = new QImage(m_fbo->toImage());

        emit imageUpdated();
    }
}

void GtRenderer::onDestroy()
{
    m_depthFbo = nullptr;
    m_fbo = nullptr;
    m_scene = nullptr;
}
