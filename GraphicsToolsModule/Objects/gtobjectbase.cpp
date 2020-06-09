#include "gtobjectbase.h"

#include "GraphicsToolsModule/gtrenderer.h"

GtDrawableBase::GtDrawableBase()
    : m_renderer(nullptr)
{

}

void GtDrawableBase::Update(const std::function<void (OpenGLFunctions*)>& f)
{
    if(m_renderer != nullptr) {
        m_renderer->Asynch([this, f]{
            f(m_renderer);
        });
    }
}

void GtDrawableBase::Update(const FAction& f)
{
    if(m_renderer != nullptr) {
        m_renderer->Asynch([f]{
            f();
        });
    }
}

void GtDrawableBase::initialize(GtRenderer* renderer)
{
    m_renderer = renderer;
    onInitialize(renderer);
}
