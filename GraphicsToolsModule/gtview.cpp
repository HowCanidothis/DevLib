#include "gtview.h"

#include "internal.hpp"
#include "ControllersModule/controllerscontainer.h"
#include "gtplayercontrollercamera.h"

GtView::GtView(QWidget* parent, Qt::WindowFlags flags)
    : QOpenGLWidget(parent, flags)
    , _initialized(false)
    , _controllers(new ControllersContainer())
{

    new GtPlayerControllerCamera("GtPlayerControllerCamera", _controllers.data());
    auto context = new GtControllersContext();

    _controllers->SetContext(context);
}

void GtView::initializeGL()
{
    LOGOUT;

    if(!initializeOpenGLFunctions()) {
        log.Error() << "Cannot initialize opengl functions";
        return;
    }

    _materialTexture = new GtMaterial();
    _materialTexture->addMesh(GtMeshQuad2D::instance(this));
    _materialTexture->addParameter(new GtMaterialParameterTexture("TextureMap", "post_render"));
    _materialTexture->setShaders(GT_SHADERS_PATH, "screen.vert", "screen.frag");
}

void GtView::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
}

void GtView::paintGL()
{
    if(!isInitialized()) {
        return;
    }

    _materialTexture->draw(this);
}
