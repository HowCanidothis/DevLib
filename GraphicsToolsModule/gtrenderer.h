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

    void SetControllers(class ControllersContainer* controllers, struct GtControllersContext* context = nullptr);

    void SetSamplesCount(qint32 samples);
    SharedPointer<guards::LambdaGuard> SetDefaultQueueNumber(qint32 queueNumber);
    // TODO. Not renderer methods
    void MouseMoveEvent(QMouseEvent* event);
    void MousePressEvent(QMouseEvent* event);
    void MouseReleaseEvent(QMouseEvent* event);
    void WheelEvent(QWheelEvent* event);
    void KeyPressEvent(QKeyEvent* event);
    void KeyReleaseEvent(QKeyEvent* event);

    void AddDrawable(GtDrawableBase* drawable);
    void RemoveDrawable(GtDrawableBase* drawable);
    void Update(const std::function<void (OpenGLFunctions*)>& handler);

    Point3F Project(const Point3F& position) const;

    class GtCamera* GetCamera() { return m_camera.get(); }

    QImage CurrentImage();

    Dispatcher OnInitialized;

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
    SharedPointer<Matrix4Resource> m_mvp;
    SharedPointer<Matrix4Resource> m_invertedMv;
    SharedPointer<Resource<Vector3F>> m_eye;
    SharedPointer<Resource<Vector3F>> m_forward;
    SharedPointer<Resource<Vector3F>> m_up;
    SharedPointer<Resource<Vector2F>> m_screenSize;
    ScopedPointer<GtCamera> m_camera;
    ScopedPointer<class GtFramebufferObjectBase> m_depthFbo;
    ScopedPointer<class QOpenGLFramebufferObject> m_fbo;
    ScopedPointer<struct GtControllersContext> m_controllersContext;
    qint32 m_queueNumber;
    qint32 m_samplesCount;

    ScopedPointer<class GtScene> m_scene;
    ScopedPointer<class ControllersContainer> m_controllers;
};

#endif // GTRENDERER_H
