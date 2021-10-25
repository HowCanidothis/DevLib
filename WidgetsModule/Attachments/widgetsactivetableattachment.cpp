#include "widgetsactivetableattachment.h"

WidgetsActiveTableViewAttachment::WidgetsActiveTableViewAttachment()
{
}

void WidgetsActiveTableViewAttachment::Attach(QTableView* tableView)
{
    tableView->viewport()->installEventFilter(GetInstance());
}

bool WidgetsActiveTableViewAttachment::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonRelease) {
        ActiveTable = reinterpret_cast<QTableView*>(watched->parent());
        auto* selectionModel = ActiveTable->selectionModel();
        if(selectionModel != nullptr) {
            HasSelection = !selectionModel->selectedIndexes().isEmpty();
        } else {
            HasSelection = false;
        }
    } else if(event->type() == QEvent::Destroy) {
        if(ActiveTable.Native() == reinterpret_cast<QTableView*>(watched->parent())) {
            ActiveTable = nullptr;
        }
    }
    return false;
}

WidgetsActiveTableViewAttachment* WidgetsActiveTableViewAttachment::GetInstance()
{
    static WidgetsActiveTableViewAttachment result;
    return &result;
}
