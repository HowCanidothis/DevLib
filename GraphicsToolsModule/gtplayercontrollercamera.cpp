#include "gtplayercontrollercamera.h"
#include "gtcamera.h"

#include <QKeyEvent>

Point2I GtPlayerControllerCamera::resolutional(const Point2I& p) const
{
    return p;
}

bool GtPlayerControllerCamera::mouseMoveEvent(QMouseEvent* event)
{
    Point2I resolutional_screen_pos = resolutional(event->pos());
    switch (event->buttons()) {
    case Qt::MiddleButton:
        ctx().Camera->rotate(_lastScreenPosition - resolutional_screen_pos);
        break;
    case Qt::RightButton:
        ctx().Camera->rotateRPE(_lastScreenPosition - resolutional_screen_pos);
        break;
    case Qt::LeftButton: {
        Vector3F dist = _lastPlanePosition - ctx().Camera->unprojectPlane(resolutional_screen_pos);
        ctx().Camera->translate(dist.x(), dist.y());
        break;
    }
    default:
        break;
    }
    _lastScreenPosition = resolutional_screen_pos;
    _lastPlanePosition = ctx().Camera->unprojectPlane(resolutional_screen_pos);
    return true;
}

bool GtPlayerControllerCamera::mousePressEvent(QMouseEvent* event)
{
    _lastScreenPosition = resolutional(event->pos());
    _lastPlanePosition = ctx().Camera->unprojectPlane(_lastScreenPosition);
    if(event->buttons() == Qt::MiddleButton) {
        ctx().Camera->setRotationPoint(_lastPlanePosition);
        return true;
    }
    return false;
}

bool GtPlayerControllerCamera::wheelEvent(QWheelEvent* event)
{
    ctx().Camera->focusBind(event->pos());
    ctx().Camera->zoom(event->delta() > 0);
    ctx().Camera->focusRelease();
    return true;
}

bool GtPlayerControllerCamera::keyReleaseEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_P: ctx().Camera->setIsometric(false); break;
    case Qt::Key_I: ctx().Camera->setIsometric(true); break;
    default: return false;
    };
    return true;
}

bool GtPlayerControllerCamera::inputHandle(const QSet<qint32>* inputKeys, qint32 modifiers)
{
    float move_dist = 50;
    if(modifiers) move_dist *= 10;
    for(qint32 key : *inputKeys){
        switch (key)
        {
            case Qt::Key_W: ctx().Camera->moveForward(move_dist); break;
            case Qt::Key_S: ctx().Camera->moveForward(-move_dist); break;
            case Qt::Key_A: ctx().Camera->moveSide(move_dist); break;
            case Qt::Key_D: ctx().Camera->moveSide(-move_dist); break;
        default:
            return false;
        }
    }
    return true;
}
