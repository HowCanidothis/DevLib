#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QScopedPointer>

#include <SharedGuiModule/internal.hpp>

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT
    typedef QOpenGLWidget Super;
public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    void StartRendering();

    // QWidget interface
protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* e) Q_DECL_OVERRIDE;

private:
    QScopedPointer<class GLRender> _render;
    QScopedPointer<class GLThread> _thread;
    ScopedPointer<class GtCamera> _camera;
    ScopedPointer<class ControllersContainer> _controllers;

    Point3F _lastPlanePosition;
    Point2I _lastScreenPosition;
};

#endif // GLWIDGET_H
