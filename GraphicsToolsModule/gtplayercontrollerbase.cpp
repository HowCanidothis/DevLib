#include "gtplayercontrollerbase.h"

#include <QKeyEvent>

bool GtPlayerControllerBase::keyReleaseEvent(QKeyEvent* e)
{
    _pressedKeys -= e->key();
    _modifiers = e->modifiers();

    return false;
}

bool GtPlayerControllerBase::keyPressEvent(QKeyEvent* e)
{
    _pressedKeys += e->key();
    _modifiers = e->modifiers();

    return false;
}
