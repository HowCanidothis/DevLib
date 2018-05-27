#include "gtplayercontrollerbase.h"

#include <QKeyEvent>

void GtPlayerControllerBase::keyReleaseEvent(QKeyEvent* e)
{
    pressed_keys -= e->key();
    modifiers = e->modifiers();
}

void GtPlayerControllerBase::keyPressEvent(QKeyEvent* e)
{
    pressed_keys += e->key();
    modifiers = e->modifiers();
}
