#include "gtrendererbase.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>

GtRendererBase::GtRendererBase(GtRendererBase* sharedRenderer)
    : m_surfaceFormat(QSurfaceFormat::defaultFormat())
    , m_context(new QOpenGLContext)
    , m_surface(new QOffscreenSurface)
    , m_isInitialized(false)
    , m_shareRenderer(sharedRenderer)
    , m_isValid(true)
{
    m_context->setFormat(m_surfaceFormat);
    if(m_shareRenderer != nullptr) {
        m_context->setShareContext(m_shareRenderer->m_context.get());
    }
    m_context->create();

    m_context->moveToThread(this);

    m_surface->setFormat(m_context->format());
    m_surface->create();
}

GtRendererBase::~GtRendererBase()
{
}

void GtRendererBase::run()
{
    if(m_shareRenderer != nullptr) {
        while(!m_shareRenderer->m_isInitialized && m_shareRenderer->m_isValid);
        if(!m_shareRenderer->m_isValid) {
            return;
        }
    }

    if(!m_context->isValid()) {
        qCCritical(LC_UI) << "Unable to initialize opengl context";
        return;
    }

    if(!m_surface->isValid()) {
        qCCritical(LC_UI) << "Unable to create offscreen surface";
        return;
    }

    qCInfo(LC_UI) << QString("OpenGL is initialized") << m_context->format();

    m_context->makeCurrent(m_surface.get());
    if(!onInitialize()) {
        m_isValid = false;
        return;
    }

    m_isInitialized = true;

    while (!IsStoped()) {
        callEvents();

        auto guard = guards::make(this, &GtRendererBase::fpsBind, &GtRendererBase::fpsRelease);
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
