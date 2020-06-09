#include "gtrendererbase.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>

GtRendererBase::GtRendererBase()
{

}

GtRendererBase::~GtRendererBase()
{
}

void GtRendererBase::Resize(qint32 w, qint32 h)
{
    Asynch([this, w, h]{
        onResize(w, h);
    });
}

void GtRendererBase::run()
{
    {
        QMutexLocker locker(&initializationMutex());

        m_context = new QOpenGLContext;
        m_context->setFormat(m_surfaceFormat);
        if(!m_context->create()) {
            qCCritical(LC_UI) << "Unable to initialize opengl context";
            return;
        }

        ThreadsBase::DoMainAwait([this]{
            m_surface = new QOffscreenSurface;
            m_surface->setFormat(m_context->format());
            m_surface->create();
        });

        if(!m_surface->isValid()) {
            qCCritical(LC_UI) << "Unable to create offscreen surface";
            return;
        }
    }

    m_context->makeCurrent(m_surface.get());
    onInitialize();
    onResize(100,100);

    while (!IsStoped()) {
        auto guard = guards::make(this, &GtRendererBase::fpsBind, &GtRendererBase::fpsRelease);

        callPauseableEvents();

        compute();
    }

    onDestroy();
}

void GtRendererBase::compute()
{
    Q_ASSERT(m_context.get() == QOpenGLContext::currentContext());
    onDraw();
}

QMutex& GtRendererBase::initializationMutex()
{
    static QMutex result;
    return result;
}
