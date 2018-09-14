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
    , _initialized(false)
    , _controllers(new ControllersContainer())
    , _camera(new GtCamera())
{
    _camera->setProjectionProperties(45.f, 10.f, 1000000.f);

    QTimer* render = new QTimer();
    connect(render, SIGNAL(timeout()), this, SLOT(update()));
    render->start(30);

    new GtPlayerControllerCamera("GtPlayerControllerCamera", _controllers.data());
    auto context = new GtControllersContext();
    context->Camera = _camera.data();

    _controllers->SetContext(context);
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

    MVP = ResourcesSystem::GetResource<Matrix4>("mvp");

//    _materialTexture = new GtMaterial();
//    _materialTexture->addMesh(GtMeshQuad2D::instance(this));
//    _materialTexture->addParameter(new GtMaterialParameterTexture("TextureMap", "post_render"));
//    _materialTexture->setShaders(GT_SHADERS_PATH, "screen.vert", "screen.frag");

    _surfaceMesh = new GtMeshGrid(50, 50, 5);
    _surfaceMesh->initialize(this);

    _surfaceMaterial = new GtMaterial();
    _surfaceMaterial->addMesh(_surfaceMesh.data());

    _surfaceMaterial->addParameter(new GtMaterialParameterMatrix("MVP", "mvp"));
    _surfaceMaterial->addParameter(new GtMaterialParameterBase("zValue", [](QOpenGLShaderProgram* program, gLocID location, OpenGLFunctions*){
        program->setUniformValue(location, 100.f);
    }));

    _surfaceMaterial->setShaders(GT_SHADERS_PATH, "colored2d.vert", "colored.frag");

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GtView::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
    _camera->resize(w, h);
}

void GtView::paintGL()
{
    if(!isInitialized()) {
        return;
    }

    MVP->Get() = _camera->getWorld();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glViewport(0,0,width(),height());

    _surfaceMaterial->draw(this);
}

void GtView::mouseMoveEvent(QMouseEvent* event)
{
    _controllers->MouseMoveEvent(event);
}

void GtView::mousePressEvent(QMouseEvent* event)
{
    _controllers->MousePressEvent(event);
}

void GtView::wheelEvent(QWheelEvent* event)
{
    _controllers->WheelEvent(event);
}

void GtView::keyPressEvent(QKeyEvent *event)
{
    _controllers->KeyPressEvent(event);
}

void GtView::keyReleaseEvent(QKeyEvent *event)
{
    _controllers->KeyReleaseEvent(event);
}

