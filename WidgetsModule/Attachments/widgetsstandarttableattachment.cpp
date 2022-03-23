#include "widgetsstandarttableattachment.h"

#include <QMenu>
#include <QTableView>

#include <ActionsModule/internal.hpp>

#include "widgetsactivetableattachment.h"
#include "WidgetsModule/TableViews/Header/widgetsresizableheaderattachment.h"

/*struct SerializedHeaderState
{
    QVector<qint32> ShownColumns;
    QVector<qint32> HiddenColumns;
    QVector<std::pair<qint32, qint32>> ColumnsLocationMap;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << ShownColumns;
        buffer << HiddenColumns;
        buffer << ColumnsLocationMap;
    }
};*/

WidgetsStandartTableHeaderManager::WidgetsStandartTableHeaderManager()
    : m_updateStates(2000)
{}

QByteArray WidgetsStandartTableHeaderManager::saveState(QHeaderView* headerView)
{
    /*SerializedHeaderState state;
    auto sectionsCount = headerView->count();
    for(auto i(0); i < sectionsCount; ++i) {
        if(headerView->isSectionHidden(i)) {
            state.HiddenColumns.append(i);
        } else {
            state.ShownColumns.append(i);
        }
        state.ColumnsLocationMap.append(std::make_pair(i, headerView->visualIndex(i)));
    }
    return SerializeToArray(state);*/
    return headerView->saveState();
}

void WidgetsStandartTableHeaderManager::restoreState(const QByteArray& array, QHeaderView* headerView)
{
    /*SerializedHeaderState state;
    DeSerializeFromArray(array, state);

    for(const auto& index : state.ShownColumns) {
        headerView->setSectionHidden(index, false);
    }
    for(const auto& index : state.HiddenColumns) {
        headerView->setSectionHidden(index, true);
    }
    for(const auto& location : state.ColumnsLocationMap) {
        headerView->moveSection(headerView->visualIndex(location.first), location.second);
    }*/
    headerView->restoreState(array);
}

void WidgetsStandartTableHeaderManager::Register(const Latin1Name& stateName, QHeaderView* headerView)
{
    if(stateName.IsNull()) {
        return;
    }

    auto foundIt = m_states.find(stateName);
    if(foundIt != m_states.end()) {
        restoreState(*foundIt.value().CurrentState, headerView);
        foundIt.value().Headers.insert(headerView);
    } else {
        foundIt = m_states.insert(stateName, State());
        *foundIt.value().CurrentState = saveState(headerView);
        foundIt.value().Headers.insert(headerView);
    }

    auto stateProperty = foundIt.value().CurrentState;

    auto updateStates = m_updateStates.Wrap([stateName, this, stateProperty]{
        for(auto* headerView : m_states[stateName].Headers) {
            restoreState(*stateProperty, headerView);
            headerView->parentWidget()->update();
        }
    });

    auto stateConnection = stateProperty->OnChanged.Connect(this, updateStates).MakeSafe();

    QObject::connect(headerView, &QHeaderView::destroyed, [stateConnection, this, headerView, stateName]{
        m_states[stateName].Headers.remove(headerView);
    });
    auto updateState = [this, headerView, stateName, stateProperty]{
        *stateProperty = saveState(headerView);
    };
    QObject::connect(headerView, &QHeaderView::sectionMoved, updateState);
    QObject::connect(headerView, &QHeaderView::sectionResized, updateState);
}

QHeaderView* WidgetsStandartTableAttachment::AttachHorizontal(QTableView* tableView, const DescColumnsParams& params)
{
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Horizontal, tableView);
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
    WidgetsStandartTableHeaderManager::GetInstance().Register(params.StateTag, dragDropHeader);
    return dragDropHeader;
}

QHeaderView* WidgetsStandartTableAttachment::AttachVertical(class QTableView* tableView, const DescColumnsParams& params)
{
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Vertical, tableView);
    tableView->setVerticalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto* editScope = ActionsManager::GetInstance().FindScope("Edit");
    if(editScope != nullptr){
        auto actions = editScope->GetActionsQList();
        tableView->addActions(actions);
    }
    tableView->addAction(dragDropHeader->CreateShowColumsMenu(nullptr, params)->menuAction());

    tableView->setWordWrap(true);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    WidgetsActiveTableViewAttachment::Attach(tableView);
    WidgetsStandartTableHeaderManager::GetInstance().Register(params.StateTag, dragDropHeader);
    return dragDropHeader;
}
