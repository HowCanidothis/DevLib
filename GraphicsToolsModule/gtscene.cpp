#include "gtscene.h"
#include "Objects/gtobjectbase.h"

GtScene::GtScene()
{
    m_initialized = false;
}

GtScene::~GtScene()
{
    for(auto* drawable : m_drawables){
        delete drawable;
    }
}

void GtScene::draw(OpenGLFunctions* f)
{
    for(auto* drawable : m_drawables){
        drawable->draw(f);
    }
}

void GtScene::AddDrawable(GtDrawableBase* drawable)
{
    Q_ASSERT(!m_drawables.contains(drawable));
    m_drawables.insert(drawable);
}

void GtScene::RemoveDrawable(GtDrawableBase* drawable)
{
    Q_ASSERT(m_drawables.contains(drawable));
    m_drawables.remove(drawable);
    delete drawable;
}
