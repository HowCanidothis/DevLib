#ifndef GTVIEW_H
#define GTVIEW_H

#include "Objects/gtmaterialparametertexturebase.h"
#include <SharedGuiModule/internal.hpp>

struct GtViewParams
{
    bool DebugMode;
};

class GtView : public QWidget
{
    using Super = QWidget;
public:
    GtView(QWidget* parent = nullptr, Qt::WindowFlags flags=0);
    ~GtView();

    void SetController(const GtRendererControllerPtr& controller);

    // QOpenGLWidget interface
protected:
    void paintEvent(QPaintEvent* ) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void keyPressEvent(QKeyEvent* e) override;
    void resizeEvent(QResizeEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    GtRendererControllerPtr m_controller;
    bool m_clicked;
    QImage m_image;
};

#endif // GTVIEW_H
