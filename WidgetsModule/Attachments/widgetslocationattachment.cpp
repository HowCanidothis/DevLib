#include "widgetslocationattachment.h"

#include <QWidget>
#include <QResizeEvent>

#include "WidgetsModule/Components/componentplacer.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

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
            if(m_target->windowFlags().testFlag(Qt::Popup)) {
                m_target->move(relativeWidget->mapToGlobal(position));
            } else {
                auto* parent = m_target->parentWidget();
                m_target->move(relativeWidget->mapTo(parent, position));
                m_target->raise();
            }

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
    static auto updateParent = [](WidgetsLocationAttachment* t, QEvent* event){
        auto size = event->type() == QEvent::Resize ? reinterpret_cast<QResizeEvent*>(event)->size() : t->m_parent->size();
//        if(t->m_componentPlacer->Location.Native() == QuadTreeF::Location_Center) {
            //t->m_target->resize(t->m_parent->width() - t->m_componentPlacer->Offset.Native().x() * 2, t->m_parent->height() - t->m_componentPlacer->Offset.Native().y() * 2);
//        }
        t->m_componentPlacer->ParentSize = size;
        t->m_componentPlacer->ResultPosition.Invoke();
    };
    static auto updateTarget = [](WidgetsLocationAttachment* t, QEvent* event){
        auto size = event->type() == QEvent::Resize ? reinterpret_cast<QResizeEvent*>(event)->size() : t->m_target->size();
//        if(t->m_componentPlacer->Location.Native() == QuadTreeF::Location_Center) {
//            QSize size(t->m_parent->width() - t->m_componentPlacer->Offset.Native().x() * 2, t->m_parent->height() - t->m_componentPlacer->Offset.Native().y() * 2);
//            if(t->m_target->size() != size) {
                //t->m_target->resize(size);
//                return true;
//            }
//        }
        t->m_componentPlacer->TargetSize = size;
        return false;
    };

    switch (event->type()) {
    case QEvent::Move: {
        if(watched == m_parent && m_target->isVisible()){
            m_componentPlacer->ResultPosition.Invoke();
        }
        break;
    }
    case QEvent::Show:
        if(watched == m_target) {
            updateParent(this, event);
            return updateTarget(this, event);
        } else if(watched == m_parent){
            updateParent(this, event);
        }
        break;
    case QEvent::Resize: {
        if(m_target->isVisible()) {
            if(watched == m_target) {
                return updateTarget(this, event);
            } else if(watched == m_parent){
                updateParent(this, event);
            } else {
                m_componentPlacer->ResultPosition.Invoke();
            }
        }
    } break;
    default:
        break;
    }
    return false;
}
