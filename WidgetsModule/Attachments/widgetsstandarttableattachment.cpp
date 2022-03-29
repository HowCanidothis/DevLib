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

void WidgetsStandartTableHeaderManager::updateState(const Latin1Name& stateName)
{
    auto foundIt = m_states.find(stateName);
    if(foundIt != m_states.end()) {
        const auto& data = foundIt.value().GetData();
        data->Update();
    }
}

void WidgetsStandartTableHeaderManager::State::Update()
{
    m_qtConnections.Clear();
    for(auto* headerView : Headers) {
        WidgetsStandartTableHeaderManager::GetInstance().restoreState(CurrentState, headerView);
        headerView->parentWidget()->update();
        Connect(headerView);
    }
}

void WidgetsStandartTableHeaderManager::State::Initialize(const Latin1Name& stateName)
{
    Q_ASSERT(m_stateName.IsNull());
    m_stateName = stateName;
    CurrentState.OnChanged.Connect(this, [this, stateName]{
        UpdateState.Call([stateName]{
            WidgetsStandartTableHeaderManager::GetInstance().updateState(stateName);
        });
    });
}

WidgetsStandartTableHeaderManager::State::State()
    : UpdateState(2000)
{}

void WidgetsStandartTableHeaderManager::State::Connect(QHeaderView* headerView)
{
    auto updateState = [this, headerView]{
        if(headerView->isVisible()) {
            CurrentState = WidgetsStandartTableHeaderManager::GetInstance().saveState(headerView);
        }
    };

    m_qtConnections.connect(headerView, &QHeaderView::sectionMoved, updateState);
}

WidgetsStandartTableHeaderManager::StateObject::StateObject()
    : m_data(::make_shared<WidgetsStandartTableHeaderManager::State>())
{}

void WidgetsStandartTableHeaderManager::StateObject::Initialize(const Latin1Name& stateName)
{
    m_data->Initialize(stateName);
}

void WidgetsStandartTableHeaderManager::Register(const Latin1Name& stateName, QHeaderView* headerView)
{
    if(stateName.IsNull()) {
        return;
    }

    auto foundIt = m_states.find(stateName);
    if(foundIt != m_states.end()) {
        restoreState(foundIt.value().GetData()->CurrentState, headerView);
        foundIt.value().GetData()->Headers.insert(headerView);
    } else {
        foundIt = m_states.insert(stateName, StateObject());
        foundIt.value().Initialize(stateName);
        const auto& data = foundIt.value().GetData();
        data->CurrentState = saveState(headerView);
        data->Headers.insert(headerView);
    }

    QObject::connect(headerView, &QHeaderView::destroyed, [this, headerView, stateName]{
        m_states[stateName].GetData()->Headers.remove(headerView);
    });

    foundIt.value().GetData()->Connect(headerView);
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
