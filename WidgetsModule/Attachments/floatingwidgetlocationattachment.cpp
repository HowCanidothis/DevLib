#include "floatingwidgetlocationattachment.h"

#include <QWidget>
#include <QResizeEvent>

#include "WidgetsModule/Components/componentplacer.h"
#include "WidgetsModule/Utils/styleutils.h"

FloatingWidgetLocationAttachment::FloatingWidgetLocationAttachment(QWidget* target, QuadTreeF::BoundingRect_Location location, const QPoint& offset)
    : m_componentPlacer(::make_scoped<ComponentPlacer>(500))
    , m_target(target)
{
    m_componentPlacer->Location = location;
    m_componentPlacer->Offset = offset;

    target->parent()->installEventFilter(this);
    target->installEventFilter(this);

    m_componentPlacer->Initialize();

    m_componentPlacer->ResultPosition.Subscribe([this]{
        m_target->move(m_componentPlacer->ResultPosition);
    });;
    StyleUtils::InstallSizeAdjuster(target);
}

bool FloatingWidgetLocationAttachment::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Resize: {
        auto* resizeEvent = reinterpret_cast<QResizeEvent*>(event);
        if(watched == m_target) {
            m_componentPlacer->TargetSize = resizeEvent->size();
        } else {
            m_componentPlacer->ParentSize = resizeEvent->size();
        }
    } break;
    default:
        break;
    }
    return false;
}
