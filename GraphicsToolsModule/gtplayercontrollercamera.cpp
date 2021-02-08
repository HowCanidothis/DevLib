#include "gtplayercontrollercamera.h"
#include "gtcamera.h"
#include "gtdepthbuffer.h"

#include <QKeyEvent>

Point2I GtPlayerControllerCamera::resolutional(const Point2I& p) const
{
    return p;
}

bool GtPlayerControllerCamera::mouseMoveEvent(QMouseEvent* event)
{
    Point2I resolutional_screen_pos = resolutional(event->pos());
    auto depth = ctx().DepthBuffer->ValueAt(resolutional_screen_pos.x(), resolutional_screen_pos.y());
    switch (event->buttons()) {
    case Qt::MiddleButton:
        ctx().Camera->RotateRPE(ctx().LastScreenPoint - resolutional_screen_pos);
        break;
    case Qt::RightButton:
        ctx().Camera->Rotate(ctx().LastScreenPoint - resolutional_screen_pos);
        break;
    case Qt::LeftButton: {
        ctx().Camera->MoveFocused(resolutional_screen_pos);
        break;
    }
    default:
        break;
    }
    ctx().LastScreenPoint = resolutional_screen_pos;
    m_lastWorldPosition = ctx().Camera->Unproject(resolutional_screen_pos.x(), resolutional_screen_pos.y(), depth);
    return true;
}

bool GtPlayerControllerCamera::mousePressEvent(QMouseEvent* event)
{
    ctx().LastScreenPoint = resolutional(event->pos());
    const auto& lastScreenPosition = ctx().LastScreenPoint;
    if(event->button() == Qt::RightButton || event->button() == Qt::LeftButton) {
        auto depth = ctx().DepthBuffer->ValueAt(lastScreenPosition.x(), lastScreenPosition.y());
        /*if(qFuzzyCompare(depth,1.f)) {
            ctx().Camera->FocusRelease();
        } else {*/
            ctx().Camera->FocusBind(lastScreenPosition, depth);
        //}
        return false;
    }

    return false;
}

bool GtPlayerControllerCamera::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::RightButton) {
        return true;
    }
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
