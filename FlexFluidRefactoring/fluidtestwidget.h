#ifndef FLUIDTESTWIDGET_H
#define FLUIDTESTWIDGET_H

#include <QOpenGLWidget>
#include "SharedGuiModule/gt_decl.h"

class FluidTestWidget : public QOpenGLWidget, protected OpenGLFunctions
{
    Q_OBJECT

    ScopedPointer<class GtCamera> camera;
    ScopedPointer<class GtFramebufferObjectMultisampled> fbo;
    ScopedPointer<class GtMeshSurface> surface_mesh;
    ScopedPointer<class GtViewBase> surface_view;
public:
    explicit FluidTestWidget(QWidget *parent = 0);
    ~FluidTestWidget();
    // QOpenGLWidget interface
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
};

#endif // FLUIDTESTWIDGET_H
