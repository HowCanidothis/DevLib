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
    GtPlayerControllerCamera(const Name& name, ControllersContainer* container, ControllerBase* parent=nullptr);

    void SetRotationButtons(const QSet<Qt::MouseButtons>& buttons) { m_rotationButtons = buttons; }
    void SetMovementButtons(const QSet<Qt::MouseButtons>& buttons) { m_movementButtons = buttons; }

    bool IsRotationButton(Qt::MouseButtons button) const { return (m_rotationButtons.isEmpty() && m_movementButtons.isEmpty()) ? RotationButtons.contains(button) : m_rotationButtons.contains(button); }
    bool IsMovementButton(Qt::MouseButtons button) const { return (m_rotationButtons.isEmpty() && m_movementButtons.isEmpty()) ? MovementButtons.contains(button) : m_movementButtons.contains(button); }

    static const QSet<Qt::MouseButtons> RotationButtons;
    static const QSet<Qt::MouseButtons> MovementButtons;

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
    QSet<Qt::MouseButtons> m_rotationButtons;
    QSet<Qt::MouseButtons> m_movementButtons;
};

#endif // GTPLAYERCONTROLLERCAMERA_H
