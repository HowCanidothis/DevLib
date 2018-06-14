#ifndef GTPLAYERCONTROLLERCAMERA_H
#define GTPLAYERCONTROLLERCAMERA_H

#include <ControllersModule/controllerbase.h>
#include "SharedGuiModule/decl.h"
#include "decl.h"

class GtCamera;

class GtPlayerControllerCamera : public ControllerContextedBase<GtControllersContext>
{
    typedef ControllerContextedBase<GtControllersContext> Super;
public:
    using Super::Super;

    // GtPlayerControllerBase interface
public:
    bool mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    bool mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    bool wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    bool keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    bool inputHandle(const QSet<qint32>* inputKeys, qint32 modifiers) Q_DECL_OVERRIDE;

private:
    Point2I resolutional(const Point2I& p) const;

protected:
    Point3F _lastPlanePosition;
    Point2I _lastScreenPosition;
};

#endif // GTPLAYERCONTROLLERCAMERA_H
