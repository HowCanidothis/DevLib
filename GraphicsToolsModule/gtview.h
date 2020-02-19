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
    GtView(ScopedPointer<GtViewParams>&& params, QWidget* parent, Qt::WindowFlags flags=0);
    ~GtView();

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
    ScopedPointer<class GtMeshQuad2D> m_meshQuad;
    ScopedPointer<class GtMaterial> m_materialTexture;
    ScopedPointer<class ControllersContainer> m_controllers;
    ScopedPointer<class GtCamera> m_camera;

    ScopedPointer<class GtMaterial> m_surfaceMaterial;
    ScopedPointer<class GtMeshBase> m_surfaceMesh;
    ScopedPointer<class GtMeshBase> m_linesMesh;
    ScopedPointer<class GtMaterial> m_linesMaterial;
    ScopedPointer<class GtFramebufferObjectBase> m_fbo;
    struct GtControllersContext* m_controllersContext;

    ScopedPointer<GtViewParams> m_params;
};

#endif // GTVIEW_H
