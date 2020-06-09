#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include <SharedModule/External/external.hpp>
#include <SharedModule/internal.hpp>

class GtObjectBase
{
public:
    virtual void MapProperties(Observer* ) {}
};

class GtDrawableBase : public GtObjectBase
{
public:
    GtDrawableBase();
    virtual ~GtDrawableBase() {}

    void Update(const std::function<void (OpenGLFunctions*)>& f);
    void Update(const FAction& f);

protected:
    friend class GtScene;
    friend class GtRenderer;
    virtual void draw(OpenGLFunctions* f) = 0;
    void initialize(class GtRenderer* renderer);
    virtual void onInitialize(OpenGLFunctions* f) = 0;

private:
    GtRenderer* m_renderer;
};


#endif // GTOBJECTBASE_H
