#ifndef GTVIEW_H
#define GTVIEW_H

#include "SharedGui/decl.h"
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
    bool initialized;

    ScopedPointer<class GtMeshQuad2D> mesh_quad;
    ScopedPointer<class GtMaterial> texture_material;
};

#endif // GTVIEW_H
