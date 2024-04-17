#ifndef GRAPHICSTOOLSMODULE_DECL_H
#define GRAPHICSTOOLSMODULE_DECL_H

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/internal.hpp>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
#include <QOpenGLFunctions_4_5_Core>
typedef QOpenGLFunctions_4_5_Core OpenGLFunctions;
#endif

namespace GtNames {
DECLARE_GLOBAL_NAME(mvp);
DECLARE_GLOBAL_NAME(screenSize)
DECLARE_GLOBAL_NAME(invertedMVP)
DECLARE_GLOBAL_NAME(eye)
DECLARE_GLOBAL_NAME(side)
DECLARE_GLOBAL_NAME(up)
DECLARE_GLOBAL_NAME(forward)
DECLARE_GLOBAL_NAME(view)
DECLARE_GLOBAL_NAME(projection)
DECLARE_GLOBAL_NAME(rotation)
DECLARE_GLOBAL_NAME(viewportProjection)
DECLARE_GLOBAL_NAME(camera)
}

struct GtControllersContext
{
    class GtCamera* Camera;
    class GtRenderer* Renderer;
    class QOpenGLFramebufferObject* FrameBuffer;
    class GtDepthBuffer* DepthBuffer;
    Point2I LastScreenPoint;
    Point3F LastWorldPoint;
};

using GtShaderProgramPtr = SharedPointer<class GtShaderProgram>;
using GtMeshPtr = SharedPointer<class GtMesh>;
using GtMeshBufferPtr = SharedPointer<class GtMeshBuffer>;
using GtRendererPtr = SharedPointer<class GtRenderer>;
using GtRendererControllerPtr = SharedPointer<class GtRendererController>;
using GtFontPtr = SharedPointer<class GtFont>;
using GtRenderProperties = QHash<Name, QVariant>;
using GtDrawableBasePtr = SharedPointer<class GtDrawableBase>;
using GtRenderPathPtr = SharedPointer<class GtRenderPath>;

struct GtDrawableDeleter
{
    void operator()(GtDrawableBase* obj);
};

inline GtDrawableBasePtr make_shared(GtDrawableBase* drawable)
{
    return GtDrawableBasePtr(drawable, GtDrawableDeleter());
}


#endif // DECL_H
