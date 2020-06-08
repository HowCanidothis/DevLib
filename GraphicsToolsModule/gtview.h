#ifndef GTVIEW_H
#define GTVIEW_H

#include <QOpenGLWidget>

#include "Objects/gtmaterialparametertexturebase.h"
#include <SharedGuiModule/internal.hpp>

struct GtViewParams
{
    bool DebugMode;
};

class GtView : public QOpenGLWidget, protected OpenGLFunctions
{
public:
    GtView(QWidget* parent = nullptr, Qt::WindowFlags flags=0);
    ~GtView();

    void Initialize(const SharedPointer<GtViewParams>& params);

    void SetScene(class GtScene* scene);
    class GtCamera* GetCamera() { return m_camera.get(); }

    // QOpenGLWidget interface
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;

private:
    bool m_isInitialized;

    ScopedPointer<Matrix4Resource> m_mvp;
    ScopedPointer<Matrix4Resource> m_invertedMvp;
    ScopedPointer<Resource<Vector3F>> m_eye;
    ScopedPointer<class ControllersContainer> m_controllers;
    ScopedPointer<class GtCamera> m_camera;
    ScopedPointer<class GtFramebufferObjectBase> m_fbo;
    ScopedPointer<class GtFramebufferObjectBase> m_depthFbo;
    struct GtControllersContext* m_controllersContext;

    SharedPointer<GtViewParams> m_params;
    class GtScene* m_scene;
};

#endif // GTVIEW_H
