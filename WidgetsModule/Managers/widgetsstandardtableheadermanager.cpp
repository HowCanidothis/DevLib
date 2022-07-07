#include "widgetsstandardtableheadermanager.h"

#include <QHeaderView>

WidgetsStandardTableHeaderManager::WidgetsStandardTableHeaderManager()
{}

QByteArray WidgetsStandardTableHeaderManager::saveState(QHeaderView* headerView)
{
    return headerView->saveState();
}

void WidgetsStandardTableHeaderManager::restoreState(const QByteArray& array, QHeaderView* headerView)
{
    headerView->restoreState(array);
}

void WidgetsStandardTableHeaderManager::updateState(const Latin1Name& stateName)
{
    auto foundIt = m_states.find(stateName);
    if(foundIt != m_states.end()) {
        const auto& data = foundIt.value().GetData();
        data->Update();
    }
}

void WidgetsStandardTableHeaderManager::State::Update()
{
    m_qtConnections.Clear();
    for(auto* headerView : Headers) {
        WidgetsStandardTableHeaderManager::GetInstance().restoreState(CurrentState, headerView);
        headerView->parentWidget()->update();
        Connect(headerView);
    }
}

void WidgetsStandardTableHeaderManager::State::Initialize(const Latin1Name& stateName)
{
    Q_ASSERT(m_stateName.IsNull());
    m_stateName = stateName;
    CurrentState.OnChanged.Connect(this, [this, stateName]{
        UpdateState.Call(CONNECTION_DEBUG_LOCATION, [stateName]{
            WidgetsStandardTableHeaderManager::GetInstance().updateState(stateName);
        });
    });
}

WidgetsStandardTableHeaderManager::State::State()
    : UpdateState(2000)
{}

void WidgetsStandardTableHeaderManager::State::Connect(QHeaderView* headerView)
{
    auto updateState = [this, headerView]{
        if(headerView->isVisible()) {
            CurrentState = WidgetsStandardTableHeaderManager::GetInstance().saveState(headerView);
        }
    };

    m_qtConnections.connect(headerView, &QHeaderView::sectionMoved, updateState);
}

WidgetsStandardTableHeaderManager::StateObject::StateObject()
    : m_data(::make_shared<WidgetsStandardTableHeaderManager::State>())
{}

void WidgetsStandardTableHeaderManager::StateObject::Initialize(const Latin1Name& stateName)
{
    m_data->Initialize(stateName);
}

void WidgetsStandardTableHeaderManager::Register(const Latin1Name& stateName, QHeaderView* headerView)
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
