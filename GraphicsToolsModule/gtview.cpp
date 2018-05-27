#include "gtview.h"

#include "internal.hpp"

GtView::GtView(QWidget* parent, Qt::WindowFlags flags)
    : QOpenGLWidget(parent, flags)
    , initialized(false)
{

}

void GtView::initializeGL()
{
    LOGOUT;

    initialized = initializeOpenGLFunctions();
    if(!initialized) {
        log.Error() << "Cannot initialize opengl functions";
        return;
    }

    texture_material = new GtMaterial();
    texture_material->addMesh(GtMeshQuad2D::instance(this));
    texture_material->addParameter(new GtMaterialParameterTexture("TextureMap", "post_render"));
    texture_material->setShaders(GT_SHADERS_PATH, "screen.vert", "screen.frag");
}

void GtView::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
}

void GtView::paintGL()
{
    texture_material->draw(this);
}
