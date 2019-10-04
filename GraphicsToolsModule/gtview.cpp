#include "gtview.h"

#include "internal.hpp"
#include <ControllersModule/controllerscontainer.h>
#include "gtplayercontrollercamera.h"
#include "Objects/gtmaterialparametermatrix.h"

#include "gtmeshgrid.h"
#include "Objects/gtmaterial.h"

#include <QTimer>

GtView::GtView(QWidget* parent, Qt::WindowFlags flags)
    : QOpenGLWidget(parent, flags)
    , m_isInitialized(false)
    , m_controllers(new ControllersContainer())
    , m_camera(new GtCamera())
{
    m_camera->SetProjectionProperties(45.f, 10.f, 1000000.f);

    QTimer* render = new QTimer();
    connect(render, SIGNAL(timeout()), this, SLOT(update()));
    render->start(30);

    new GtPlayerControllerCamera("GtPlayerControllerCamera", m_controllers.data());
    auto context = new GtControllersContext();
    context->Camera = m_camera.data();

    m_controllers->SetContext(context);
}
#include <QOpenGLShaderProgram>
void GtView::initializeGL()
{
    if(!initializeOpenGLFunctions()) {
        qCInfo(LC_SYSTEM) << "Cannot initialize opengl functions";
        return;
    }

    ResourcesSystem::RegisterResource("mvp", [this]{
        return new Matrix4();
    });

    m_mvp = ResourcesSystem::GetResource<Matrix4>("mvp");

//    _materialTexture = new GtMaterial();
//    _materialTexture->addMesh(GtMeshQuad2D::instance(this));
//    _materialTexture->addParameter(new GtMaterialParameterTexture("TextureMap", "post_render"));
//    _materialTexture->setShaders(GT_SHADERS_PATH, "screen.vert", "screen.frag");

    m_surfaceMesh = new GtMeshGrid(50, 50, 5);
    m_surfaceMesh->Initialize(this);

    m_surfaceMaterial = new GtMaterial();
    m_surfaceMaterial->AddMesh(m_surfaceMesh.data());

    m_surfaceMaterial->AddParameter(new GtMaterialParameterMatrix("MVP", "mvp"));
    m_surfaceMaterial->AddParameter(new GtMaterialParameterBase("zValue", [](QOpenGLShaderProgram* program, gLocID location, OpenGLFunctions*){
        program->setUniformValue(location, 100.f);
    }));

    m_surfaceMaterial->SetShaders(GT_SHADERS_PATH, "colored2d.vert", "colored.frag");

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GtView::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
    m_camera->Resize(w, h);
}

void GtView::paintGL()
{
    if(!isInitialized()) {
        return;
    }

    m_mvp->Data().Set(m_camera->GetWorld());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glViewport(0,0,width(),height());

    m_surfaceMaterial->Draw(this);
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

