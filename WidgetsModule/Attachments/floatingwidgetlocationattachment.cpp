#include "floatingwidgetlocationattachment.h"

#include <QWidget>
#include <QResizeEvent>

#include "WidgetsModule/Components/componentplacer.h"
#include "WidgetsModule/Utils/styleutils.h"

FloatingWidgetLocationAttachment::FloatingWidgetLocationAttachment(QWidget* target, QuadTreeF::BoundingRect_Location location, const QPoint& offset, QWidget* relativeWidget, qint32 delay)
    : m_componentPlacer(::make_scoped<ComponentPlacer>(delay))
    , m_target(target)
{
    m_componentPlacer->Location = location;
    m_componentPlacer->Offset = offset;

    target->installEventFilter(this);
    target->parent()->installEventFilter(this);

    if(relativeWidget != nullptr && relativeWidget != m_target->parentWidget()) {
        m_parent = relativeWidget;
        relativeWidget->installEventFilter(this);
        m_componentPlacer->Initialize();

        m_componentPlacer->ResultPosition.Subscribe([this, relativeWidget]{
            auto* parent = m_target->parentWidget();
            m_target->move(relativeWidget->mapTo(parent, m_componentPlacer->ResultPosition));
            m_target->raise();
        });
    } else {
        m_parent = target->parentWidget();
        m_componentPlacer->Initialize();

        m_componentPlacer->ResultPosition.Subscribe([this]{
            m_target->move(m_componentPlacer->ResultPosition);
        });;
    }
    StyleUtils::InstallSizeAdjuster(target);
}

bool FloatingWidgetLocationAttachment::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Resize: {
        auto* resizeEvent = reinterpret_cast<QResizeEvent*>(event);
        if(watched == m_target) {
            m_componentPlacer->TargetSize = resizeEvent->size();
        } else if(watched == m_parent){
            m_componentPlacer->ParentSize = resizeEvent->size();
        } else {
            m_componentPlacer->ResultPosition.Invoke();
        }
    } break;
    default:
        break;
    }
    return false;
}
