#ifndef GTPLAYERCONTROLLERCAMERA_H
#define GTPLAYERCONTROLLERCAMERA_H

#include "gtplayercontrollerbase.h"
#include "SharedGuiModule/decl.h"

class GtCamera;

class GtPlayerControllerCamera : public GtPlayerControllerBase
{
    typedef GtPlayerControllerBase Super;
public:
    using GtPlayerControllerBase::GtPlayerControllerBase;

    // GtPlayerControllerBase interface
public:
    bool mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    bool mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    bool wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    bool keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    bool inputHandle() Q_DECL_OVERRIDE;

private:
    Point2I resolutional(const Point2I& p) const;

protected:
    Point3F _lastPlanePosition;
    Point2I _lastScreenPosition;
};

#endif // GTPLAYERCONTROLLERCAMERA_H
