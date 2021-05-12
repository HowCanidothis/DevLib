#include "widgetsactivetableattachment.h"

WidgetsActiveTableViewAttachment::WidgetsActiveTableViewAttachment()
    : m_activeTable("Application/ActiveEditTable", PropertiesSystem::Global)
    , m_hasSelection("Application/ActiveEditTableHasSelection", PropertiesSystem::Global)
{
}

void WidgetsActiveTableViewAttachment::Attach(QTableView* tableView)
{
    tableView->viewport()->installEventFilter(instance());
}

bool WidgetsActiveTableViewAttachment::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonRelease) {
        auto* tv = reinterpret_cast<QTableView*>(watched->parent());
        m_activeTable = tv;
        auto* selectionModel = m_activeTable->selectionModel();
        if(selectionModel != nullptr) {
            m_hasSelection = !selectionModel->selectedIndexes().isEmpty();
        } else {
            m_hasSelection = false;
        }
    } else if(event->type() == QEvent::Destroy) {
        if(m_activeTable == reinterpret_cast<QTableView*>(watched->parent())) {
            m_activeTable = nullptr;
        }
    }
    return false;
}

WidgetsActiveTableViewAttachment* WidgetsActiveTableViewAttachment::instance()
{
    static WidgetsActiveTableViewAttachment result;
    return &result;
}
