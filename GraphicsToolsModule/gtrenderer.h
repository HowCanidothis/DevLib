#ifndef GTRENDERER_H
#define GTRENDERER_H

#include <SharedGuiModule/internal.hpp>
#include <ResourcesModule/internal.hpp>

#include "gtrendererbase.h"
#include <GraphicsToolsModule/Objects/gtmaterialparameterbase.h>

class GtRenderer : public GtRendererBase, protected OpenGLFunctions
{
    Q_OBJECT
public:
    GtRenderer(const PropertiesScopeName& scopeName);
    ~GtRenderer();

    void SetControllers(class ControllersContainer* controllers);

    // TODO. Not renderer methods
    void MouseMoveEvent(QMouseEvent* event);
    void MousePressEvent(QMouseEvent* event);
    void MouseReleaseEvent(QMouseEvent* event);
    void WheelEvent(QWheelEvent* event);
    void KeyPressEvent(QKeyEvent* event);
    void KeyReleaseEvent(QKeyEvent* event);

    void AddDrawable(GtDrawableBase* drawable);

    QImage CurrentImage();

signals:
    void imageUpdated();

    // GtRenderThread interface
protected:
    void onInitialize() override;
    void onResize(qint32 w, qint32 h) override;
    void onDraw() override;
    void onDestroy() override;

private:
    friend class GtDrawableBase;
    QMutex m_outputImageMutex;

    ScopedPointer<QImage> m_outputImage;
    ScopedPointer<Matrix4Resource> m_mvp;
    ScopedPointer<Matrix4Resource> m_invertedMv;
    ScopedPointer<Resource<Vector3F>> m_eye;
    ScopedPointer<Resource<Vector3F>> m_forward;
    ScopedPointer<class GtCamera> m_camera;
    ScopedPointer<class GtFramebufferObjectBase> m_depthFbo;
    ScopedPointer<QOpenGLFramebufferObject> m_fbo;
    ScopedPointer<struct GtControllersContext> m_controllersContext;

    ScopedPointer<class GtScene> m_scene;
    ScopedPointer<class ControllersContainer> m_controllers;
};

#endif // GTRENDERER_H
