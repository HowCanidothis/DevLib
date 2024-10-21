#include "widgetsactivetableattachment.h"

#include <QHeaderView>
#include <QApplication>
#include <QTableView>
#include <QAbstractButton>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsActiveTableViewAttachment::WidgetsActiveTableViewAttachment()
{
    HasSelection.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](qint32 count){
        return count != 0;
    }, SelectedRowsCount);
    ActiveTable.Connect(CONNECTION_DEBUG_LOCATION, [this](QTableView* tv){
        if(tv == nullptr) {
            SelectedRowsCount = 0;
            return;
        }

        auto* selectionModel = tv->selectionModel();
        if(selectionModel != nullptr) {
            SelectedRowsCount = WidgetTableViewWrapper(tv).SelectedRowsSet().size();
        } else {
            SelectedRowsCount = 0;
        }
    });
}

void WidgetsActiveTableViewAttachment::updateActiveTableView(QTableView* tableView)
{
    ActiveTable = tableView;
}

void WidgetsActiveTableViewAttachment::Attach(QTableView* tableView)
{
    tableView->installEventFilter(GetInstance());
    tableView->viewport()->installEventFilter(GetInstance());
    auto update = [tableView]{
        GetInstance()->ActiveTable = tableView;
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
    case QEvent::FocusIn:
    case QEvent::MouseButtonRelease: {
        QTableView* tv = qobject_cast<QTableView*>(watched);
        ActiveTable = tv ? tv : reinterpret_cast<QTableView*>(watched->parent());
        break;
    }
    case QEvent::FocusOut:
    case QEvent::Destroy: {
        QTableView* tv = qobject_cast<QTableView*>(watched);
        tv = tv ? tv : reinterpret_cast<QTableView*>(watched->parent());
        if(ActiveTable.Native() == tv){
            ActiveTable = nullptr;
        }
        break;
    }
    default: break;
    }
    return false;
}

WidgetsActiveTableViewAttachment* WidgetsActiveTableViewAttachment::GetInstance()
{
    static WidgetsActiveTableViewAttachment result;
    return &result;
}
