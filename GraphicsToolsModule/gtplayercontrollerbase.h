#ifndef GTPLAYERCONTROLLERBASE_H
#define GTPLAYERCONTROLLERBASE_H

#include <QSet>
#include <ControllersModule/controllerbase.h>
#include "decl.h"

class GtPlayerControllerBase : public ControllerBase
{    
public:
    using ControllerBase::ControllerBase;

    virtual bool keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    virtual bool keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;

protected:
    GtControllersContext& ctx() { return context<GtControllersContext>(); }
    const GtControllersContext& ctx() const { return context<GtControllersContext>(); }

protected:
    qint32 _modifiers;
    QSet<qint32> _pressedKeys;
};

#endif // GTPLAYERCONTROLLERBASE_H
