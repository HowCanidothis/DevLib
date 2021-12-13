#include "widgetsactivetableattachment.h"

#include <QHeaderView>

WidgetsActiveTableViewAttachment::WidgetsActiveTableViewAttachment()
{
}

void WidgetsActiveTableViewAttachment::updateActiveTableView(QTableView* tableView)
{
    ActiveTable = tableView;
    auto* selectionModel = ActiveTable->selectionModel();
    if(selectionModel != nullptr) {
        HasSelection = !selectionModel->selectedIndexes().isEmpty();
    } else {
        HasSelection = false;
    }
}

void WidgetsActiveTableViewAttachment::Attach(QTableView* tableView)
{
    tableView->viewport()->installEventFilter(GetInstance());

    auto update = [tableView]{
        GetInstance()->updateActiveTableView(tableView);
    };
    connect(tableView->horizontalHeader(), &QHeaderView::sectionClicked, update);
    connect(tableView->verticalHeader(), &QHeaderView::sectionClicked, update);
}

bool WidgetsActiveTableViewAttachment::eventFilter(QObject* watched, QEvent* event)
{
    switch(event->type()) {
    case QEvent::MouseButtonRelease: {
        updateActiveTableView(reinterpret_cast<QTableView*>(watched->parent()));
        break;
    }
    case QEvent::Destroy:
        if(ActiveTable.Native() == watched->parent()) {
            ActiveTable = nullptr;
        }
        break;
    default: break;
    }
    return false;
}

WidgetsActiveTableViewAttachment* WidgetsActiveTableViewAttachment::GetInstance()
{
    static WidgetsActiveTableViewAttachment result;
    return &result;
}
