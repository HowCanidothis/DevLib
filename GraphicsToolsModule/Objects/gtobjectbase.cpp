#include "gtobjectbase.h"

#include "GraphicsToolsModule/gtrenderer.h"

GtDrawableBase::GtDrawableBase(GtRenderer* renderer)
    : m_renderer(renderer)
{
    m_renderer->AddDrawable(this);
}

void GtDrawableBase::Update(const std::function<void (OpenGLFunctions*)>& f)
{
    m_renderer->Asynch([this, f]{
        f(m_renderer);
    });
}

void GtDrawableBase::Update(const FAction& f)
{
    m_renderer->Asynch([f]{
        f();
    });
}

void GtDrawableBase::initialize(GtRenderer* renderer)
{
    onInitialize(renderer);
}
