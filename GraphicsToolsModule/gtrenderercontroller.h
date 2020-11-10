#ifndef GTRENDERERCONTROLLER_H
#define GTRENDERERCONTROLLER_H

#include <SharedModule/internal.hpp>

class GtRendererController : public QObject
{
    Q_OBJECT
public:
    GtRendererController(class GtRenderer* renderer, class ControllersContainer* controllersContainer, struct GtControllersContext* context);

    void SetProjectionProperties(float angle, float nearValue, float farValue);

    void Resize(qint32 w, qint32 h);
    void MouseMoveEvent(QMouseEvent* event);
    void MousePressEvent(QMouseEvent* event);
    void MouseReleaseEvent(QMouseEvent* event);
    void WheelEvent(QWheelEvent* event);
    void KeyPressEvent(QKeyEvent* event);
    void KeyReleaseEvent(QKeyEvent* event);

    class GtCamera* GetCamera() { return m_camera.get(); }
    QImage GetCurrentImage() const;
    double GetRenderTime() const { return m_renderTime; }

signals:
    void imageUpdated();

private:
    void setCurrentImage(QImage* image, double renderTime);
    void onInitialize();
    void onDestroy();

private:
    friend class GtRenderer;
    mutable QMutex m_outputImageMutex;
    GtRenderer* m_renderer;
    ScopedPointer<QImage> m_outputImage;
    ScopedPointer<GtCamera> m_camera;
    ScopedPointer<GtControllersContext> m_controllersContext;
    ScopedPointer<ControllersContainer> m_controllers;

    ScopedPointer<class GtFramebufferObjectBase> m_depthFbo;
    ScopedPointer<class QOpenGLFramebufferObject> m_fbo;
    double m_renderTime;
};

#endif // GTRENDERERCONTROLLER_H
