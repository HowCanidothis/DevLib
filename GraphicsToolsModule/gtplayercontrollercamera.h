#ifndef GTPLAYERCONTROLLERCAMERA_H
#define GTPLAYERCONTROLLERCAMERA_H

#include "gtplayercontrollerbase.h"
#include "SharedGui/decl.h"

class GtCamera;

class GtPlayerControllerCamera : public GtPlayerControllerBase
{
    typedef GtPlayerControllerBase Super;
    GtCamera* camera;
    Point3F last_plane_position;
    Point2I last_screen_position;
public:
    GtPlayerControllerCamera();

    void setCamera(GtCamera* camera) { this->camera = camera; }
    // GtPlayerControllerBase interface
public:
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void inputHandle() Q_DECL_OVERRIDE;
private:
    Point2I resolutional(const Point2I& p) const;
};

#endif // GTPLAYERCONTROLLERCAMERA_H
