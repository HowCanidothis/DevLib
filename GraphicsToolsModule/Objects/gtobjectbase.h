#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include <SharedModule/External/external.hpp>
#include <SharedModule/internal.hpp>

#include "GraphicsToolsModule/decl.h"

class GtObjectBase
{
public:
    virtual ~GtObjectBase(){}
    virtual void MapProperties(QtObserver* ) {}
};

class GtDrawableBase : public GtObjectBase
{
public:
    GtDrawableBase(class GtRenderer* renderer);
    ~GtDrawableBase();

    AsyncResult Destroy();
    AsyncResult Update(const std::function<void (OpenGLFunctions*)>& f);
    AsyncResult Update(const FAction& f);

    ThreadHandler CreateThreadHandler();
    ThreadHandlerNoThreadCheck CreateThreadNoCheckHandler();

protected:
    friend class GtScene;
    friend class GtRenderer;
    friend class GtRendererController;
    virtual void drawDepth(OpenGLFunctions* f) { draw(f); }
    virtual void draw(OpenGLFunctions* f) = 0;
    void initialize(class GtRenderer* renderer);
    virtual void onInitialize(OpenGLFunctions* f) = 0;
    virtual void onDestroy(OpenGLFunctions* f) = 0;

    void enableDepthTest();
    void disableDepthTest();
    const GtRenderProperties& getRenderProperties() const;

protected:
    GtRenderer* m_renderer;
    std::atomic_bool m_destroyed;
    bool m_rendererDrawable;
};

inline void GtDrawableDeleter::operator()(GtDrawableBase* obj)
{
    obj->Destroy();
}

#endif // GTOBJECTBASE_H
