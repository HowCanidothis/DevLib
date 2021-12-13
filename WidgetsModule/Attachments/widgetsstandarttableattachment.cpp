#include "widgetsstandarttableattachment.h"

#include <QMenu>
#include <QTableView>

#include <ActionsModule/internal.hpp>

#include "widgetsactivetableattachment.h"
#include "WidgetsModule/TableViews/Header/widgetsresizableheaderattachment.h"

void WidgetsStandartTableAttachment::Attach(QTableView* tableView, const DescColumnsParams& params)
{
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(tableView);
    tableView->setHorizontalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto* editScope = ActionsManager::GetInstance().FindScope("Edit");
    if(editScope != nullptr){
        auto actions = editScope->GetActionsQList();
        tableView->addActions(actions);
    }
    tableView->addAction(dragDropHeader->CreateShowColumsMenu(nullptr, params)->menuAction());

    tableView->setWordWrap(true);
    auto* verticalHeader = tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    WidgetsActiveTableViewAttachment::Attach(tableView);
}
