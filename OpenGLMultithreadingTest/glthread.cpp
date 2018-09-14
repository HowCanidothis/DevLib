#include "glthread.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QMutexLocker>

#include "glrender.h"

GLThread::GLThread(GLRender* render, QOpenGLWidget* parent)
    : QThread(parent)
    , _render(render)
{

}

void GLThread::Resize(qint32 w, qint32 h)
{
    Asynch([this, w, h]{
        _render->Resize(w, h);
    });
}

void GLThread::Asynch(const std::function<void ()>& event)
{
    QMutexLocker lock(&_eventsLocker);
    _events.append(event);
}

void GLThread::Start()
{
    _stoped = false;
    Super::start();
}

void GLThread::Stop()
{
    _stoped = true;
    Super::wait();
}

void GLThread::run()
{
    auto context = getGL()->context();

    getGL()->makeCurrent();

    _render->Initialize();

    while(!_stoped) {

        getGL()->makeCurrent();

        {
            QMutexLocker lock(&_eventsLocker);
            for(const auto& event : _events) {
                event();
            }
        }

        _render->Draw();

        context->swapBuffers(context->surface());
    }

    getGL()->doneCurrent();
    context->moveToThread(getGL()->thread());
}

QOpenGLWidget*GLThread::getGL() const
{
    return reinterpret_cast<QOpenGLWidget*>(parent());
}
