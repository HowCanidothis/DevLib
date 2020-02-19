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
    switch (event->buttons()) {
    case Qt::MiddleButton:
        ctx().Camera->Rotate(m_lastScreenPosition - resolutional_screen_pos);
        break;
    case Qt::RightButton:
        ctx().Camera->RotateRPE(m_lastScreenPosition - resolutional_screen_pos);
        break;
    case Qt::LeftButton: {
        Vector3F dist = m_lastPlanePosition - ctx().Camera->UnprojectPlane(resolutional_screen_pos);
        ctx().Camera->Translate(dist.x(), dist.y());
        break;
    }
    default:
        break;
    }
    m_lastScreenPosition = resolutional_screen_pos;
    m_lastPlanePosition = ctx().Camera->UnprojectPlane(resolutional_screen_pos);
    return true;
}

bool GtPlayerControllerCamera::mousePressEvent(QMouseEvent* event)
{
    m_lastScreenPosition = resolutional(event->pos());
    m_lastPlanePosition = ctx().Camera->UnprojectPlane(m_lastScreenPosition);
    if(event->buttons() == Qt::MiddleButton) {
        ctx().Camera->SetRotationPoint(m_lastPlanePosition);
        return true;
    }

    return false;
}

bool GtPlayerControllerCamera::wheelEvent(QWheelEvent* event)
{
    ctx().Camera->FocusBind(event->pos());
    ctx().Camera->Zoom(event->delta() > 0);
    ctx().Camera->FocusRelease();
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
