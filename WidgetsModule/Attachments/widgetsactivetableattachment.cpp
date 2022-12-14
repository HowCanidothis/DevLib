#include "widgetsactivetableattachment.h"

#include <QHeaderView>
#include <QTableView>
#include <QAbstractButton>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsActiveTableViewAttachment::WidgetsActiveTableViewAttachment()
{
    HasSelection.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](qint32 count){
        return count != 0;
    }, &SelectedRowsCount);
}

void WidgetsActiveTableViewAttachment::updateActiveTableView(QTableView* tableView)
{
    ActiveTable = tableView;
    auto* selectionModel = ActiveTable->selectionModel();
    if(selectionModel != nullptr) {
        SelectedRowsCount = WidgetTableViewWrapper(tableView).SelectedRowsSet().size();
    } else {
        SelectedRowsCount = 0;
    }
}

void WidgetsActiveTableViewAttachment::Attach(QTableView* tableView)
{
    tableView->viewport()->installEventFilter(GetInstance());
    auto update = [tableView]{
        GetInstance()->updateActiveTableView(tableView);
    };

    if(auto button = tableView->findChild<QAbstractButton*>(QString(), Qt::FindDirectChildrenOnly)) {
        button->setToolTip(tr("Select All"));
        connect(button, &QAbstractButton::clicked, update);
    }
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
