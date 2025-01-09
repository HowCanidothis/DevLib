#include "widgetsstandardtableheadermanager.h"

#include <QHeaderView>

#include "WidgetsModule/Utils/widgethelpers.h"

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
    CurrentState.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this, stateName]{
        UpdateState.Call(CONNECTION_DEBUG_LOCATION, [this, stateName]{
            Update();
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
    m_qtConnections.connect(headerView, &QHeaderView::sectionResized, updateState);
    m_qtConnections.connect(headerView, &QHeaderView::sortIndicatorChanged, updateState);
}

WidgetsStandardTableHeaderManager::StateObject::StateObject()
    : m_data(::make_shared<WidgetsStandardTableHeaderManager::State>())
{}

void WidgetsStandardTableHeaderManager::StateObject::Initialize(const Latin1Name& stateName)
{
    m_data->Initialize(stateName);
}

void WidgetsStandardTableHeaderManager::Register(const DescTableViewParams& params, QHeaderView* headerView)
{
    const auto& stateName = params.StateTag;
    WidgetHeaderViewWrapper header(headerView);
    QVector<std::pair<qint32, qint32>> fixedSizes;
    for(auto it(params.ColumnsParams.begin()), e(params.ColumnsParams.end()); it != e; ++it) {
         if(it.value().FixedSize != -1) {
             fixedSizes.append(std::make_pair(it.key(), it.value().FixedSize));
         }
    }

    auto applyParams = [&]{
        Q_ASSERT(params.ColumnsParams.isEmpty() || header->count() != 0);
        for(auto it(params.ColumnsParams.begin()), e(params.ColumnsParams.end()); it != e; ++it) {
             header.SectionVisibility(it.key()) = it.value().Visible;
             if(it.value().ReplacePlaceTo != -1) {
                header.MoveSection(it.key(), it.value().ReplacePlaceTo);
             }
        }
    };

    guards::LambdaGuard guard([&fixedSizes, headerView]{
        if(!fixedSizes.isEmpty()) {
            WidgetWrapper(headerView).AddEventFilter([fixedSizes, headerView](QObject*, QEvent* e) {
                switch(e->type()) {
                case QEvent::StyleChange:
                case QEvent::Show:
                    for(auto [logicalIndex, size] : fixedSizes) {
                        headerView->resizeSection(logicalIndex, size);
                        headerView->setSectionResizeMode(logicalIndex, QHeaderView::Fixed);
                    } break;
                default: break;
                }
                return false;
            });
        }
    });

    if(stateName.IsNull()) {
        applyParams();
        return;
    }

    auto foundIt = m_states.find(stateName);
    if(foundIt != m_states.end()) {
        restoreState(foundIt.value().GetData()->CurrentState, headerView);
        foundIt.value().GetData()->Headers.insert(headerView);
    } else {
        applyParams();
        foundIt = m_states.insert(stateName, StateObject());
        foundIt.value().Initialize(stateName);
        const auto& data = foundIt.value().GetData();
        data->CurrentState.EditSilent() = saveState(headerView);
        data->Headers.insert(headerView);
    }

    QObject::connect(headerView, &QHeaderView::destroyed, [this, headerView, stateName]{
        m_states[stateName].GetData()->Headers.remove(headerView);
    });

    auto data = foundIt.value().GetData();
    auto eventFilterObject = ::make_shared<EventFilterObject*>(nullptr);
    *eventFilterObject = WidgetWrapper(headerView).AddEventFilter([fixedSizes, headerView, data, eventFilterObject](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::ShowToParent:
            data->Connect(headerView);
            WidgetsStandardTableHeaderManager::GetInstance().restoreState(data->CurrentState, headerView);
            (*eventFilterObject)->deleteLater();
        default: break;
        }
        return false;
    });
}
