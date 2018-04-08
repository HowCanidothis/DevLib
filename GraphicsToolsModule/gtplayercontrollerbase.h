#ifndef GTPLAYERCONTROLLERBASE_H
#define GTPLAYERCONTROLLERBASE_H

#include <QSet>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

class GtPlayerControllerBase
{
protected:
    bool control_modifier;
    QSet<qint32> pressed_keys;
public:
    virtual void mouseMoveEvent(QMouseEvent*){}
    virtual void mousePressEvent(QMouseEvent*){}
    virtual void wheelEvent(QWheelEvent*){}
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void inputHandle() {}
};

#endif // GTPLAYERCONTROLLERBASE_H
