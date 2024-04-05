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
    bool mouseMoveEvent(QMouseEvent* event) override;
    bool mousePressEvent(QMouseEvent* event) override;
    bool mouseReleaseEvent(QMouseEvent* event) override;
    bool wheelEvent(QWheelEvent* event) override;
    bool keyReleaseEvent(QKeyEvent* event) override;
    bool inputHandle(const QSet<qint32>* inputKeys, qint32 modifiers) override;

protected:
    Point2I resolutional(const Point2I& p) const;
};

#endif // GTPLAYERCONTROLLERCAMERA_H
