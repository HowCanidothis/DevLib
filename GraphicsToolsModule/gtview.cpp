#include "gtview.h"

#include <QTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLDebugLogger>

#include <ControllersModule/internal.hpp>

#include "Objects/gtmaterialparametermatrix.h"
#include "Objects/gtmaterial.h"
#include "Objects/gtmaterialparametertexture.h"
#include "gtframebufferobject.h"
#include "gtmeshgrid.h"
#include "gtcamera.h"
#include "gtmeshquad2D.h"
#include "gtplayercontrollercamera.h"
#include "gtdepthbuffer.h"
#include "gtscene.h"

GtView::GtView(const SharedPointer<GtViewParams>& params, QWidget* parent, Qt::WindowFlags flags)
    : QOpenGLWidget(parent, flags)
    , m_isInitialized(false)
    , m_controllers(new ControllersContainer())
    , m_camera(new GtCamera())
    , m_params(std::move(params))
{
    m_camera->SetProjectionProperties(45.f, 0.1f, 1000000.f);

    QTimer* render = new QTimer();
    connect(render, SIGNAL(timeout()), this, SLOT(update()));
    render->start(30);

    new GtPlayerControllerCamera("GtPlayerControllerCamera", m_controllers.data());
    m_controllersContext = new GtControllersContext();
    m_controllersContext->Camera = m_camera.data();
    m_controllersContext->DepthBuffer = new GtDepthBuffer(this);

    m_controllers->SetContext(m_controllersContext);

    if(m_params->DebugMode) {
        QSurfaceFormat format = this->format();
        format.setOption(QSurfaceFormat::DebugContext);
        setFormat(format);
    }

    setMouseTracking(true);
}

GtView::~GtView()
{

}

void GtView::SetScene(GtScene* scene)
{
    m_scene = scene;
}

void GtView::initializeGL()
{
    if(!initializeOpenGLFunctions()) {
        qCInfo(LC_SYSTEM) << "Cannot initialize opengl functions";
        return;
    }

    ResourcesSystem::RegisterResource("mvp", []{
        return new Matrix4();
    });

    ResourcesSystem::RegisterResource("invertedMvp", []{
        return new Matrix4();
    });

    ResourcesSystem::RegisterResource("eye", []{
        return new Vector3F();
    });

    if(m_params->DebugMode) {
        auto* logger = new QOpenGLDebugLogger(this);
        if(logger->initialize()) {
            logger->startLogging();
        }
    }

    m_mvp = ResourcesSystem::GetResource<Matrix4>("mvp");
    m_eye = ResourcesSystem::GetResource<Vector3F>("eye");
    m_invertedMvp = ResourcesSystem::GetResource<Matrix4>("invertedMvp");

    // TODO. Must have state machine feather
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_NONE);

    glLineWidth(5.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.7f, 0.7f, 1.f);

    m_scene->initialize(this);
}

void GtView::resizeGL(int w, int h)
{    
    GtFramebufferFormat fboFormat;
    fboFormat.SetDepthAttachment(GtFramebufferFormat::RenderBuffer);
    fboFormat.AddColorAttachment(GtFramebufferTextureFormat(GL_TEXTURE_2D, GL_RGBA8));
    auto fbo = new GtFramebufferObjectMultisampled(this, {w,h}, 8);
    fbo->Create(fboFormat);
    m_fbo = fbo;

    GtFramebufferFormat depthFboFormat;
    depthFboFormat.SetDepthAttachment(GtFramebufferFormat::Texture);

    auto depthFbo = new GtFramebufferObject(this, {w,h});
    depthFbo->Create(depthFboFormat);
    m_depthFbo = depthFbo;

    m_controllersContext->DepthBuffer->SetFrameBuffer(depthFbo, context());

    m_camera->Resize(w,h);
}

void GtView::paintGL()
{
    if(!isInitialized()) {
        return;
    }

    m_mvp->Data().Set(m_camera->GetWorld());
    m_eye->Data().Set(m_camera->GetEye());
    m_invertedMvp->Data().Set(m_camera->GetWorldInverted());

    m_fbo->Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene->draw(this);

    m_fbo->Release();

    m_depthFbo->Bind();

    glClear(GL_DEPTH_BUFFER_BIT);

    m_scene->draw(this);

    m_depthFbo->Release();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->GetID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    glBlitFramebuffer(0,0,m_fbo->GetWidth(),m_fbo->GetHeight(),0,0,this->width(), this->height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GtView::mouseMoveEvent(QMouseEvent* event)
{
    m_controllers->MouseMoveEvent(event);
}

void GtView::mousePressEvent(QMouseEvent* event)
{
    m_controllers->MousePressEvent(event);
}

void GtView::wheelEvent(QWheelEvent* event)
{
    m_controllers->WheelEvent(event);
}

void GtView::keyPressEvent(QKeyEvent *event)
{
    m_controllers->KeyPressEvent(event);
}

void GtView::keyReleaseEvent(QKeyEvent *event)
{
    m_controllers->KeyReleaseEvent(event);
}

