#include "gtplayercontrollerbase.h"

#include <QKeyEvent>

void GtPlayerControllerBase::keyReleaseEvent(QKeyEvent* e)
{
    pressed_keys -= e->key();
    control_modifier = e->modifiers() == Qt::ControlModifier;
}

void GtPlayerControllerBase::keyPressEvent(QKeyEvent* e)
{
    pressed_keys += e->key();
    control_modifier = e->modifiers() == Qt::ControlModifier;
}
