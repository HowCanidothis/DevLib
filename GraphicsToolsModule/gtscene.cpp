#include "gtscene.h"
#include "Objects/gtobjectbase.h"

GtScene::GtScene()
{
    m_initialized = false;
}

GtScene::~GtScene()
{
    for(const auto& set : m_drawables){
        for(auto* drawable : set) {
            delete drawable;
        }
    }
}

void GtScene::draw(OpenGLFunctions* f)
{
    for(const auto& set : m_drawables){
        for(auto* drawable : set) {
            drawable->draw(f);
        }
    }
}

void GtScene::drawDepth(OpenGLFunctions* f)
{
    for(const auto& set : m_drawables){
        for(auto* drawable : set) {
            drawable->drawDepth(f);
        }
    }
}

void GtScene::AddDrawable(GtDrawableBase* drawable, qint32 queueNumber)
{
    auto foundIt = m_drawables.find(queueNumber);
    if(foundIt == m_drawables.end()) {
        foundIt = m_drawables.insert(queueNumber, {});
    }
    Q_ASSERT(!foundIt->contains(drawable));
    foundIt->insert(drawable);
}

void GtScene::RemoveDrawable(GtDrawableBase* drawable)
{
    for(auto& queue : m_drawables) {
        auto foundIt = queue.find(drawable);
        if(foundIt != queue.end()) {
            queue.erase(foundIt);
        }
    }

    delete drawable;
}
