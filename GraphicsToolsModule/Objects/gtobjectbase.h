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

    void Destroy();
    void Update(const std::function<void (OpenGLFunctions*)>& f);
    void Update(const FAction& f);

protected:
    friend class GtScene;
    friend class GtRenderer;
    virtual void drawDepth(OpenGLFunctions* f) { draw(f); }
    virtual void draw(OpenGLFunctions* f) = 0;
    void initialize(class GtRenderer* renderer);
    virtual void onInitialize(OpenGLFunctions* f) = 0;
    virtual void onDestroy(OpenGLFunctions* f) = 0;

protected:
    GtRenderer* m_renderer;
};

using GtDrawableBasePtr = SharedPointer<GtDrawableBase>;

#endif // GTOBJECTBASE_H
