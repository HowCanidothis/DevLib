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
    m_camera->SetProjectionProperties(45.f, 10.f, 1000000.f);

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

    ResourcesSystem::RegisterResource("mvp", [this]{
        return new Matrix4();
    });

    if(m_params->DebugMode) {
        auto* logger = new QOpenGLDebugLogger(this);
        if(logger->initialize()) {
            logger->startLogging();
        }
    }

    m_mvp = ResourcesSystem::GetResource<Matrix4>("mvp");

    /* m_materialTexture = new GtMaterial();
    m_materialTexture->AddMesh(GtMeshQuad2D::Instance(this));
    m_materialTexture->AddParameter(new GtMaterialParameterTexture("TextureMap", "post_render"));
    m_materialTexture->SetShaders(GT_SHADERS_PATH, "screen.vert", "screen.frag");*/

    m_surfaceMesh = new GtMeshGrid(50, 50, 5);
    m_surfaceMesh->Initialize(this);

    m_surfaceMaterial = new GtMaterial(GL_TRIANGLE_STRIP);
    m_surfaceMaterial->AddMesh(m_surfaceMesh.data());

    m_linesMesh = new GtMeshGrid(50, 50, 5);
    m_linesMesh->Initialize(this);

    m_linesMaterial = new GtMaterial(GL_LINES);
    m_linesMaterial->AddMesh(m_linesMesh.data());

    m_linesMaterial->AddParameter(new GtMaterialParameterMatrix("MVP", "mvp"));
    m_linesMaterial->AddParameter(new GtMaterialParameterBase("zValue", [](QOpenGLShaderProgram* program, gLocID location, OpenGLFunctions*){
        program->setUniformValue(location, 100.f);
    }));

    m_linesMaterial->SetShaders(GT_SHADERS_PATH, "colored2d.vert", "red.frag");

    m_surfaceMaterial->AddParameter(new GtMaterialParameterMatrix("MVP", "mvp"));
    m_surfaceMaterial->AddParameter(new GtMaterialParameterBase("zValue", [](QOpenGLShaderProgram* program, gLocID location, OpenGLFunctions*){
        program->setUniformValue(location, 0.f);
    }));

    m_surfaceMaterial->SetShaders(GT_SHADERS_PATH, "colored2d.vert", "green.frag");

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    glLineWidth(10.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.7f, 0.7f, 1.f);
}

void GtView::resizeGL(int w, int h)
{    
    GtFramebufferFormat fbo_format;
    fbo_format.SetDepthAttachment(GtFramebufferFormat::Texture);
    fbo_format.AddColorAttachment(GtFramebufferTextureFormat(GL_TEXTURE_2D, GL_RGBA8));
    auto fbo = new GtFramebufferObject(this, {w,h});
    fbo->Create(fbo_format);
    m_fbo = fbo;

    m_controllersContext->DepthBuffer->SetFrameBuffer(fbo);

    m_camera->Resize(w,h);
}

void GtView::paintGL()
{
    if(!isInitialized()) {
        return;
    }

    m_mvp->Data().Set(m_camera->GetWorld());

    m_fbo->Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_linesMaterial->Draw(this);
    m_surfaceMaterial->Draw(this);

    m_fbo->Release();

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

