#ifndef GTVIEW_H
#define GTVIEW_H

#include <QOpenGLWidget>

#include "Objects/gtmaterialparametertexturebase.h"
#include <SharedGuiModule/decl.h>

class GtView : public QOpenGLWidget, protected OpenGLFunctions
{
public:
    GtView(QWidget* parent, Qt::WindowFlags flags=0);

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
    bool _initialized;

    ScopedPointer<Matrix4Resource> MVP;
    ScopedPointer<class GtMeshQuad2D> _meshQuad;
    ScopedPointer<class GtMaterial> _materialTexture;
    ScopedPointer<class ControllersContainer> _controllers;
    ScopedPointer<class GtCamera> _camera;

    ScopedPointer<class GtMaterial> _surfaceMaterial;
    ScopedPointer<class GtMeshBase> _surfaceMesh;
};

#endif // GTVIEW_H
