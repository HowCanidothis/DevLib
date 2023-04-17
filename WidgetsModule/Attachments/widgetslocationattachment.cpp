#include "widgetslocationattachment.h"

#include <QWidget>
#include <QResizeEvent>

#include "WidgetsModule/Components/componentplacer.h"
#include "WidgetsModule/Utils/styleutils.h"

WidgetsLocationAttachment::WidgetsLocationAttachment(QWidget* target, const DescWidgetsLocationAttachmentParams& params)
    : Super(target)
    , m_componentPlacer(::make_scoped<ComponentPlacer>(params.Delay))
    , m_target(target)
{
    m_componentPlacer->Location = params.Location;
    m_componentPlacer->Offset = params.Offset;

    m_target->installEventFilter(this);
    m_target->parent()->installEventFilter(this);

    auto* relativeWidget = params.RelativeParent;

    if(relativeWidget != nullptr && relativeWidget != m_target->parentWidget()) {
        m_parent = relativeWidget;
        relativeWidget->installEventFilter(this);
        m_componentPlacer->Initialize();

        m_componentPlacer->ResultPosition.Connect(CONNECTION_DEBUG_LOCATION, [this, relativeWidget](const auto& position){
            auto* parent = m_target->parentWidget();
            m_target->move(relativeWidget->mapTo(parent, position));
            m_target->raise();
        }).MakeSafe(m_componentPlacerConnections);

        m_qtConnections.connect(relativeWidget, &QWidget::destroyed, [this]{
            m_componentPlacerConnections.clear();
        });

    } else {
        m_parent = m_target->parentWidget();
        m_componentPlacer->Initialize();

        m_componentPlacer->ResultPosition.Connect(CONNECTION_DEBUG_LOCATION, [this](const auto& position){
            m_target->move(position);
        }).MakeSafe(m_componentPlacerConnections);
    }
    StyleUtils::InstallSizeAdjuster(m_target);
}

bool WidgetsLocationAttachment::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Move: {
        if(watched == m_parent){
            m_componentPlacer->ResultPosition.Invoke();
        }
        break;
    }
    case QEvent::Resize: {
        auto* resizeEvent = reinterpret_cast<QResizeEvent*>(event);
        if(watched == m_target) {
            if(m_componentPlacer->Location.Native() == QuadTreeF::Location_Center) {
                QSize size(m_parent->width() - m_componentPlacer->Offset.Native().x() * 2, m_parent->height() - m_componentPlacer->Offset.Native().y() * 2);
                if(m_target->size() != size) {
                    m_target->resize(size);
                    return true;
                }
            }
            m_componentPlacer->TargetSize = resizeEvent->size();
        } else if(watched == m_parent){
            if(m_componentPlacer->Location.Native() == QuadTreeF::Location_Center) {
                m_target->resize(m_parent->width() - m_componentPlacer->Offset.Native().x() * 2, m_parent->height() - m_componentPlacer->Offset.Native().y() * 2);
            }
            m_componentPlacer->ParentSize = resizeEvent->size();
            m_componentPlacer->ResultPosition.Invoke();
        } else {
            m_componentPlacer->ResultPosition.Invoke();
        }
    } break;
    default:
        break;
    }
    return false;
}
