#include "widgetsstandarttableattachment.h"

#include <QMenu>
#include <QTableView>

#include <ActionsModule/internal.hpp>

#include "widgetsactivetableattachment.h"
#include "WidgetsModule/TableViews/Header/widgetsresizableheaderattachment.h"

void WidgetsStandartTableAttachment::Attach(QTableView* tableView)
{
    WidgetsActiveTableViewAttachment::Attach(tableView);
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(tableView);
    tableView->setHorizontalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto* editScope = ActionsManager::GetInstance().FindScope("Edit");
    if(editScope != nullptr){
        tableView->addActions(editScope->GetActionsQList());
    }

    tableView->setWordWrap(true);
    auto* verticalHeader = tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
}

void WidgetsStandartTableAttachment::AttachWithShowHide(QTableView* tableView, const QSet<qint32>& ignorColumns)
{
    WidgetsStandartTableAttachment::Attach(tableView);
    auto* dragDropHeader = reinterpret_cast<WidgetsResizableHeaderAttachment*>(tableView->horizontalHeader());
    tableView->addAction(dragDropHeader->CreateShowColumsMenu(nullptr, ignorColumns)->menuAction());
}
