#include "gtobjectbase.h"

#include "GraphicsToolsModule/gtrenderer.h"

GtDrawableBase::GtDrawableBase(GtRenderer* renderer)
    : m_renderer(renderer)
{
}

GtDrawableBase::~GtDrawableBase()
{
}

void GtDrawableBase::Update(const std::function<void (OpenGLFunctions*)>& f)
{
    m_renderer->Asynch([this, f]{
        f(m_renderer);
    });
}

void GtDrawableBase::Update(const FAction& f)
{
    if(QThread::currentThread() == m_renderer) {
        f();
    } else {
        m_renderer->Asynch([f]{
            f();
        });
    }
}

void GtDrawableBase::initialize(GtRenderer* renderer)
{
    onInitialize(renderer);
}
