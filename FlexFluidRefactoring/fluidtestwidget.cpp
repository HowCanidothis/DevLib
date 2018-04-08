#include "fluidtestwidget.h"
#include "GraphicsToolsModule/gtframebufferobject.h"
#include "GraphicsToolsModule/gtcamera.h"
#include "GraphicsToolsModule/gtmeshsurface.h"
#include "GraphicsToolsModule/Objects/gtviewbase.h"
#include <QOpenGLShaderProgram>

#include <QOpenGLDebugLogger>

FluidTestWidget::FluidTestWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(5);
    format.setOption(QSurfaceFormat::DebugContext);
}

FluidTestWidget::~FluidTestWidget()
{
}

void FluidTestWidget::initializeGL()
{
    if(initializeOpenGLFunctions()) {
        qWarning() << "initalize opengl functions failed";
        return;
    }

    //initialize log
    QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
    connect(logger, &QOpenGLDebugLogger::messageLogged, [=](const QOpenGLDebugMessage& message){
        qWarning() << message.type() << message.message();
    });

    if(logger && logger->initialize()) {
        logger->startLogging();
    }

    //mesh

    surface_mesh = new GtMeshSurface(3000, 2400, 320);
    surface_mesh->initialize(this);

    surface_view = new GtViewBase();
    surface_view->addMesh(surface_mesh.data());
    /*surface_view->addMaterial(new GtViewDelegateBase("MVP", [this](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions*) {
        program->setUniformValue(loc, camera->getWorld());
    }));*/

    /*{
        DirBinder dir(GT_SHADERS_PATH "Depth/");
        surface_view->setShaders("vertex2d.vert", "red.frag");
    }*/

    glPointSize(10.f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_DEPTH_TEST);
}

void FluidTestWidget::resizeGL(int w, int h)
{
    GtFramebufferFormat fbo_format;
    fbo_format.setDepthAttachment(GtFramebufferFormat::RenderBuffer);
    fbo_format.addColorAttachment(GtFramebufferTextureFormat(GL_TEXTURE_2D, GL_RGBA8));
    auto fbo = new GtFramebufferObjectMultisampled(this, {w,h}, 4);
    fbo->create(fbo_format);
    this->fbo.reset(fbo);

    camera->resize(w,h);
}

void FluidTestWidget::paintGL()
{

}
