#include "gtplayercontrollercamera.h"
#include "gtcamera.h"
#include "gtdepthbuffer.h"

#include <QKeyEvent>

const QSet<Qt::MouseButtons> GtPlayerControllerCamera::RotationButtons({Qt::LeftButton });
const QSet<Qt::MouseButtons> GtPlayerControllerCamera::MovementButtons({Qt::RightButton, Qt::MiddleButton });

GtPlayerControllerCamera::GtPlayerControllerCamera(const Name& name, ControllersContainer* container, ControllerBase* parent)
    : Super(name, container, parent)
{

}

Point2I GtPlayerControllerCamera::resolutional(const Point2I& p) const
{
    return p;
}

bool GtPlayerControllerCamera::mouseMoveEvent(QMouseEvent* event)
{
    Point2I resolutional_screen_pos = resolutional(event->pos());
    auto depth = ctx().DepthBuffer->ValueAt(resolutional_screen_pos.x(), resolutional_screen_pos.y());
    if(RotationButtons.contains(event->buttons())) {
        ctx().Camera->Rotate(ctx().LastScreenPoint - resolutional_screen_pos);
    } else if(MovementButtons.contains(event->buttons())){
        ctx().Camera->MoveFocused(resolutional_screen_pos);
    }
    ctx().LastScreenPoint = resolutional_screen_pos;
    ctx().LastWorldPoint = ctx().Camera->Unproject(resolutional_screen_pos.x(), resolutional_screen_pos.y(), depth);
    return true;
}

bool GtPlayerControllerCamera::mousePressEvent(QMouseEvent* event)
{
    ctx().LastScreenPoint = resolutional(event->pos());
    const auto& lastScreenPosition = ctx().LastScreenPoint;
    auto depth = ctx().DepthBuffer->ValueAt(lastScreenPosition.x(), lastScreenPosition.y());
    ctx().Camera->FocusBind(lastScreenPosition, depth);
    return false;
}

bool GtPlayerControllerCamera::mouseReleaseEvent(QMouseEvent* event)
{
    return false;
}

bool GtPlayerControllerCamera::wheelEvent(QWheelEvent* event)
{
    if(event->buttons() == Qt::NoButton) {
        auto depth = ctx().DepthBuffer->ValueAt(ctx().LastScreenPoint.x(), ctx().LastScreenPoint.y());
        ctx().Camera->FocusBind(ctx().LastScreenPoint, depth);
        ctx().Camera->Zoom(event->delta() > 0);
    }
    return true;
}

bool GtPlayerControllerCamera::keyReleaseEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_P: ctx().Camera->SetIsometric(false); break;
    case Qt::Key_I: ctx().Camera->SetIsometric(true); break;
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
            case Qt::Key_W: ctx().Camera->MoveForward(move_dist); break;
            case Qt::Key_S: ctx().Camera->MoveForward(-move_dist); break;
            case Qt::Key_A: ctx().Camera->MoveSide(move_dist); break;
            case Qt::Key_D: ctx().Camera->MoveSide(-move_dist); break;
        default:
            return false;
        }
    }
    return true;
}
