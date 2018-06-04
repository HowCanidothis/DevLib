#ifndef GTVIEW_H
#define GTVIEW_H

#include "SharedGuiModule/decl.h"
#include <QOpenGLWidget>

class GtView : public QOpenGLWidget, protected OpenGLFunctions
{
public:
    GtView(QWidget* parent, Qt::WindowFlags flags=0);

    // QOpenGLWidget interface
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    bool _initialized;

    ScopedPointer<class GtMeshQuad2D> _meshQuad;
    ScopedPointer<class GtMaterial> _materialTexture;
    ScopedPointer<class ControllersContainer> _controllers;
    ScopedPointer<class GtCamera> _camera;
};

#endif // GTVIEW_H
