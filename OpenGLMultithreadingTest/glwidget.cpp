#include "glwidget.h"
#include <QSurfaceFormat>
#include <QResizeEvent>

#include <ControllersModule/controllerscontainer.h>
#include <GraphicsToolsModule/gtcamera.h>
#include <GraphicsToolsModule/gtplayercontrollercamera.h>

#include "glrender.h"
#include "glthread.h"

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , _render(new GLRender())
    , _thread(new GLThread(_render.data(), this))
    , _camera(new GtCamera)
    , _controllers(new ControllersContainer)
{
    connect(_render.data(), SIGNAL(imageUpdated()), this, SLOT(update()));

    setMouseTracking(true);

    QSurfaceFormat format = this->format();
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    setFormat(format);

    _camera->setProjectionProperties(45.f,1.f,15000.f);

    _camera->setSceneBox(BoundingBox(Point3F(-10000.f,-10000.f, 3.f), Point3F(10000.f,10000.f,10000.f)));
    _camera->setIsometric(false);
    _camera->setPosition({0.f,0.f,100.f}, {0.f,0.f,-1.f}, {0.f,1.f,0.f});

    _render->SetCamera(_camera.data());

    new GtPlayerControllerCamera("CameraController", _controllers.data(), nullptr);

    auto context = new GtControllersContext();
    context->Camera = _camera.data();

    _controllers->SetContext(context);
}

GLWidget::~GLWidget()
{
    _thread->Stop();
}

void GLWidget::StartRendering()
{
    doneCurrent();
    context()->moveToThread(_thread.data());

    _thread->Start();
}

void GLWidget::paintEvent(QPaintEvent* )
{
    _controllers->Input();
}

void GLWidget::resizeEvent(QResizeEvent* e)
{
    _thread->Stop();
    Super::resizeEvent(e);
    StartRendering();

    auto eSize = e->size();
    _camera->resize(eSize.width(), eSize.height());
    _thread->Resize(eSize.width(), eSize.height());
}

void GLWidget::closeEvent(QCloseEvent* event)
{
    _thread->Stop();
    Super::closeEvent(event);
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    _controllers->MouseMoveEvent(event);
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    _controllers->MousePressEvent(event);
}

void GLWidget::wheelEvent(QWheelEvent* event)
{
    _controllers->WheelEvent(event);
}

void GLWidget::keyPressEvent(QKeyEvent* e)
{
    _controllers->KeyPressEvent(e);
}

void GLWidget::keyReleaseEvent(QKeyEvent* e)
{
    _controllers->KeyReleaseEvent(e);
}
