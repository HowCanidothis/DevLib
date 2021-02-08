#ifndef GRAPHICSTOOLSMODULE_DECL_H
#define GRAPHICSTOOLSMODULE_DECL_H

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/internal.hpp>

struct GtControllersContext
{
    class GtCamera* Camera;
    class GtRenderer* Renderer;
    class QOpenGLFramebufferObject* FrameBuffer;
    class GtDepthBuffer* DepthBuffer;
    Point2I LastScreenPoint;
};

using GtShaderProgramPtr = SharedPointer<class GtShaderProgram>;
using GtMeshPtr = SharedPointer<class GtMesh>;
using GtMeshBufferPtr = SharedPointer<class GtMeshBuffer>;
using GtRendererPtr = SharedPointer<class GtRenderer>;
using GtRendererControllerPtr = SharedPointer<class GtRendererController>;
using GtFontPtr = SharedPointer<class GtFont>;
using GtRenderProperties = QHash<Name, QVariant>;
using GtDrawableBasePtr = SharedPointer<class GtDrawableBase>;

struct GtDrawableDeleter
{
    void operator()(GtDrawableBase* obj);
};

inline GtDrawableBasePtr make_shared(GtDrawableBase* drawable)
{
    return GtDrawableBasePtr(drawable, GtDrawableDeleter());
}


#endif // DECL_H
