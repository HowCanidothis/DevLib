#include "widgetsactivetableattachment.h"

#include <QHeaderView>
#include <QApplication>
#include <QTableView>
#include <QAbstractButton>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsActiveTableViewAttachment::WidgetsActiveTableViewAttachment()
    : m_contextMenuRequest(false)
{
    HasSelection.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](qint32 count){
        return count != 0;
    }, SelectedRowsCount);
    ActiveTable.Connect(CONNECTION_DEBUG_LOCATION, [this](QTableView* tv){
        if(tv == nullptr) {
            SelectedRowsCount = 0;
            return;
        }

        updateActiveTableView(tv);
    });
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
    tableView->installEventFilter(GetInstance());
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
    case QEvent::ContextMenu:
        m_contextMenuRequest = true;
        break;
    case QEvent::FocusIn:
    case QEvent::MouseButtonRelease: {
        QTableView* tv = qobject_cast<QTableView*>(watched);
        updateActiveTableView(tv ? tv : reinterpret_cast<QTableView*>(watched->parent()));
        break;
    }
    case QEvent::FocusOut:
        if(m_contextMenuRequest){
            m_contextMenuRequest = false;
            break;
        }
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
