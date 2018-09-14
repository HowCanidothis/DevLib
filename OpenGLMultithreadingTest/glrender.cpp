#include "glrender.h"
#include <GraphicsToolsModule/gtcamera.h>

GLRender::GLRender(QObject* parent)
    : QObject(parent)
    , _camera(nullptr)
{

}

void GLRender::SetCamera(GtCamera* camera)
{
    _camera = camera;
}

void GLRender::Resize(qint32 w, qint32 h)
{
    _w = w;
    _h = h;
}

void GLRender::Initialize()
{
    initializeOpenGLFunctions();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPointSize(10.f);
    glClearColor(1.f, 0.f, 0.f, 1.f);
}

void GLRender::Draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, _w, _h);

    glMatrixMode(GL_MODELVIEW);
    Matrix4 matrix = _camera->getWorld();
    glLoadMatrixf(matrix.constData());

    glColor3f(0.f,0.f,0.f);

    static float p(0.f);
    p += 1.f;

    glBegin(GL_TRIANGLES);
    glVertex2d(p, 0.f);
    glVertex2d(p - 200.5f, 200.5f);
    glVertex2d(p - 200.5f, -200.5f);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(100.f, 0.f, 0.f);
    glVertex3f(0.f,0.f,100.f);
    glVertex3f(100.f,0.f,100.f);

    glVertex3f(0.f,200.f,100.f);
    glVertex3f(100.f,200.f,100.f);

    glVertex3f(0.f, 200.f, 0.f);
    glVertex3f(100.f, 200.f, 0.f);

    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(100.f, 0.f, 0.f);
    // Degenerate
    glVertex3f(100.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 100.f);
    // Forward pike
    glColor3f(0.f,0.f,0.f);
    glVertex3f(0.f, 0.f, 100.f);
    glVertex3f(-100.f, 100.f, 50.f);


    glColor3f(1.f,0.f,1.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(-100.f, 100.f, 50.f);


    glColor3f(1.f,1.f,0.f);

    glVertex3f(0.f, 200.f, 0.f);
    glVertex3f(-100.f, 100.f, 50.f);


    glColor3f(1.f,1.f,1.f);
    glVertex3f(0.f, 200.f, 100.f);
    glVertex3f(-100.f, 100.f, 50.f);
    glVertex3f(0.f, 0.f, 100.f);
//    // Back pike like forward


    glEnd();

    emit imageUpdated();
}
