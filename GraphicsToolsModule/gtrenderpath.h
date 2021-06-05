#ifndef GTRENDERPATH_H
#define GTRENDERPATH_H

#include "decl.h"

class GtRenderPath
{
public:
    GtRenderPath(GtRenderer* f);
    virtual ~GtRenderPath();

    virtual void Initialize() = 0;
    virtual void Render(class GtScene* scene, qint32 outputFBO) = 0;
    virtual void Resize(qint32 width, qint32 height, qint32 samples) = 0;

protected:
    void disableDepthTest();
    void enableDepthTest();

protected:
    GtRenderer* m_renderer;
    OpenGLFunctions* f;
};

class GtDefaultRenderPath : public GtRenderPath
{
    using Super = GtRenderPath;
public:
    GtDefaultRenderPath(GtRenderer* renderer);

    virtual void Initialize(){}
    virtual void Render(class GtScene* scene, qint32);
    virtual void Resize(qint32, qint32, qint32){}
};

#endif // GTRENDERPATH_H
