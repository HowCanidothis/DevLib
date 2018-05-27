#include "gtplayercontrollercamera.h"
#include "gtcamera.h"

#include <QKeyEvent>

GtPlayerControllerCamera::GtPlayerControllerCamera()
    : camera(nullptr)
{

}

Point2I GtPlayerControllerCamera::resolutional(const Point2I& p) const
{
    return p;
}

void GtPlayerControllerCamera::mouseMoveEvent(QMouseEvent* event)
{
    Point2I resolutional_screen_pos = resolutional(event->pos());
    if(event->buttons() == Qt::MiddleButton) {
        camera->rotate(last_screen_position - resolutional_screen_pos);
    }
    else if(event->buttons() == Qt::RightButton) {
        camera->rotateRPE(last_screen_position - resolutional_screen_pos);
    }
    else {
        Vector3F dist = last_plane_position - camera->unprojectPlane(resolutional_screen_pos);
        camera->translate(dist.x(), dist.y());
    }
    last_screen_position = resolutional_screen_pos;
    last_plane_position = camera->unprojectPlane(resolutional_screen_pos);
}

void GtPlayerControllerCamera::mousePressEvent(QMouseEvent* event)
{
    last_screen_position = resolutional(event->pos());
    last_plane_position = camera->unprojectPlane(last_screen_position);
    if(event->buttons() == Qt::MiddleButton) {
        camera->setRotationPoint(last_plane_position);
    }
}

void GtPlayerControllerCamera::wheelEvent(QWheelEvent* event)
{
    camera->focusBind(event->pos());
    camera->zoom(event->delta() > 0);
    camera->focusRelease();
}

void GtPlayerControllerCamera::keyReleaseEvent(QKeyEvent* e)
{
    Super::keyReleaseEvent(e);
    switch(e->key())
    {
    case Qt::Key_P: camera->setIsometric(false); break;
    case Qt::Key_I: camera->setIsometric(true); break;
    default: break;
    };
}

void GtPlayerControllerCamera::inputHandle()
{
    float move_dist = 50;
    if(modifiers) move_dist *= 10;
    for(qint32 key : pressed_keys){
        switch (key)
        {
            case Qt::Key_W: camera->moveForward(move_dist); break;
            case Qt::Key_S: camera->moveForward(-move_dist); break;
            case Qt::Key_A: camera->moveSide(move_dist); break;
            case Qt::Key_D: camera->moveSide(-move_dist); break;
        default:
            break;
        }
    }
}
