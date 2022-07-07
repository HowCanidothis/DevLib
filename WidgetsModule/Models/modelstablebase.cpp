#include "modelstablebase.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

ViewModelsTableBase::ViewModelsTableBase(QObject* parent)
    : Super(parent)
    , m_mostLeftColumnToUpdate(-1)
    , m_mostRightColumnToUpdate(-1)
{

}

ViewModelsTableBase::~ViewModelsTableBase()
{

}

qint32 ViewModelsTableBase::columnCount(const QModelIndex&) const
{
    return ColumnComponents.GetColumnCount();
}

void ViewModelsTableBase::RequestUpdateUi(qint32 left, qint32 right)
{
    if(m_mostLeftColumnToUpdate == -1) {
        m_mostLeftColumnToUpdate = left;
    } else {
        m_mostLeftColumnToUpdate = std::min(m_mostLeftColumnToUpdate, left);
    }

    if(m_mostRightColumnToUpdate == -1) {
        m_mostRightColumnToUpdate = right;
    } else {
        m_mostRightColumnToUpdate = std::max(m_mostRightColumnToUpdate, right);
    }
    m_update.Call(CONNECTION_DEBUG_LOCATION, [this]{
        emit dataChanged(createIndex(0, m_mostLeftColumnToUpdate), createIndex(rowCount()-1, m_mostRightColumnToUpdate));
        emit headerDataChanged(Qt::Horizontal, m_mostLeftColumnToUpdate, m_mostRightColumnToUpdate);
        m_mostLeftColumnToUpdate = -1;
        m_mostRightColumnToUpdate = -1;
    });
}

#ifdef UNITS_MODULE_LIB
DispatcherConnection ViewModelsTableBase::AttachTempDependence(const Name& unitName, int first, int last)
{
    return AttachTempDependence(&MeasurementManager::GetInstance().GetMeasurement(unitName)->OnChanged, first, last);
}

void ViewModelsTableBase::AttachDependence(const Name& unitName, int first, int last)
{
    AttachDependence(&MeasurementManager::GetInstance().GetMeasurement(unitName)->OnChanged, first, last);
}
#endif

DispatcherConnection ViewModelsTableBase::AttachTempDependence(Dispatcher* dispatcher, int first, int last)
{
    return dispatcher->Connect(this, [first, last, this]{
        RequestUpdateUi(first, last);
    });
}

void ViewModelsTableBase::AttachDependence(Dispatcher* dispatcher, int first, int last)
{
    AttachTempDependence(dispatcher, first, last).MakeSafe(m_connections);
}

Qt::ItemFlags ViewModelsTableBase::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::ItemIsDropEnabled;
    }
    auto componentsResult = ColumnComponents.GetFlags(index);
    if(componentsResult.has_value()) {
        return componentsResult.value() | Qt::ItemIsDragEnabled;
    }

    return Super::flags(index) | Qt::ItemIsDragEnabled;
}

QVariant ViewModelsTableBase::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    auto componentsResult = ColumnComponents.GetData(index, role);
    if(componentsResult.has_value()) {
        return componentsResult.value();
    }

    auto foundIt = m_roleDataHandlers.find(role);
    if(foundIt == m_roleDataHandlers.end()) {
        return QVariant();
    }
    return foundIt.value()(index.row(), index.column());
}

bool ViewModelsTableBase::setData(const QModelIndex& index, const QVariant& data, qint32 role)
{
    if(!index.isValid()) {
        return false;
    }
    auto componentsResult = ColumnComponents.SetData(index, data, role);
    if(componentsResult.has_value()) {
        return componentsResult.value();
    }
    auto foundIt = m_roleSetDataHandlers.find(role);
    if(foundIt == m_roleSetDataHandlers.end()) {
        return false;
    }
    return foundIt.value()(index.row(), index.column(), data);
}

QVariant ViewModelsTableBase::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const
{
    if(orientation == Qt::Horizontal) {

        auto componentsResult = ColumnComponents.GetHeaderData(section, role);
        if(componentsResult.has_value()) {
            return componentsResult.value();
        }

        auto foundIt = m_roleHorizontalHeaderDataHandlers.find(role);
        if(foundIt == m_roleHorizontalHeaderDataHandlers.end()) {
            return QVariant();
        }
        return foundIt.value()(section);
    } else {
        auto foundIt = m_roleVerticalHeaderDataHandlers.find(role);
        if(foundIt == m_roleVerticalHeaderDataHandlers.end()) {
            if(role == Qt::DisplayRole) {
                return QString::number(section + 1);
            } else if(role == Qt::TextAlignmentRole) {
                return Qt::AlignCenter;
            }
            return QVariant();
        }
        return foundIt.value()(section);
    }
}

ModelsIconsContext::ModelsIconsContext()
    : ErrorIcon(IconsManager::GetInstance().GetIcon("ErrorIcon"))
    , WarningIcon(IconsManager::GetInstance().GetIcon("WarningIcon"))
    , InfoIcon(IconsManager::GetInstance().GetIcon("InfoIcon"))
{

}

ViewModelsTableColumnComponents::ViewModelsTableColumnComponents()
{

}

void ViewModelsTableColumnComponents::AddComponent(Qt::ItemDataRole role, qint32 column, const ColumnComponentData& columnData)
{
    auto foundIt = m_columnComponents.find(column);
    if(foundIt == m_columnComponents.end()) {
        foundIt = m_columnComponents.insert(column, {});
    }
    foundIt.value()[role].append(columnData);
}

void ViewModelsTableColumnComponents::AddFlagsComponent(qint32 column, const ColumnFlagsComponentData& flagsColumnData)
{
    auto foundIt = m_columnFlagsComponents.find(column);
    if(foundIt == m_columnFlagsComponents.end()) {
        foundIt = m_columnFlagsComponents.insert(column, {});
    }
    foundIt.value().append(flagsColumnData);
}

std::optional<bool> ViewModelsTableColumnComponents::SetData(const QModelIndex& index, const QVariant& data, qint32 role)
{
    std::optional<bool> result;
    auto testHandler = [&](const QVector<ColumnComponentData>& components){
        for(const auto& handlers : adapters::reverse(components)) {
            result = handlers.SetterHandler(index, data);
            if(result.has_value()) {
                break;
            }
        }
    };

    if(callHandler(index.column(), (Qt::ItemDataRole)role, testHandler)) {
        return result;
    }
    return std::nullopt;
}

std::optional<QVariant> ViewModelsTableColumnComponents::GetData(const QModelIndex& index, qint32 role) const
{
    std::optional<QVariant> result;
    auto testHandler = [&](const QVector<ColumnComponentData>& components){
        for(const auto& handlers : adapters::reverse(components)) {
            result = handlers.GetterHandler(index);
            if(result.has_value()) {
                break;
            }            
        }
    };

    if(callHandler(index.column(), (Qt::ItemDataRole)role, testHandler)) {
        return result;
    }
    return std::nullopt;
}

std::optional<QVariant> ViewModelsTableColumnComponents::GetHeaderData(qint32 section, qint32 role) const
{
    std::optional<QVariant> result;
    auto testHandler = [&](const QVector<ColumnComponentData>& data){
        for(const auto& handlers : adapters::reverse(data)) {
            result = handlers.GetHeaderHandler();
            if(result.has_value()) {
                break;
            }            
        }
    };

    if(callHandler(section, (Qt::ItemDataRole)role, testHandler)) {
        return result;
    }
    return std::nullopt;
}

std::optional<Qt::ItemFlags> ViewModelsTableColumnComponents::GetFlags(const QModelIndex& index) const
{
    std::optional<Qt::ItemFlags> result;
    auto testHandler = [&](const QVector<ColumnFlagsComponentData>& data){
        for(const auto& handlers : adapters::reverse(data)) {
            result = handlers.GetFlagsHandler(index.row());
            if(result.has_value()) {
                break;
            }
        }
    };

    if(callFlagsHandler(index.column(), testHandler)) {
        return result;
    }
    return std::nullopt;
}

qint32 ViewModelsTableColumnComponents::GetColumnCount() const
{
    if(m_columnComponents.isEmpty()) {
        return 0;
    }
    return m_columnComponents.lastKey() + 1;
}

bool ViewModelsTableColumnComponents::callHandler(qint32 column, Qt::ItemDataRole role, const std::function<void (const QVector<ColumnComponentData> &)> &onFound) const
{
    auto foundIt = m_columnComponents.find(column);
    if(foundIt != m_columnComponents.end()) {
        const auto& columnsHandlers = foundIt.value();
        auto foundItRole = columnsHandlers.find(role);
        if(foundItRole != columnsHandlers.end()) {
            onFound(foundItRole.value());
            return true;
        }
    }
    foundIt = m_columnComponents.find(-1);
    if(foundIt != m_columnComponents.end()) {
        const auto& columnsHandlers = foundIt.value();
        auto foundItRole = columnsHandlers.find(role);
        if(foundItRole != columnsHandlers.end()) {
            onFound(foundItRole.value());
            return true;
        }
    }
    return false;
}

bool ViewModelsTableColumnComponents::callFlagsHandler(qint32 column, const std::function<void (const QVector<ColumnFlagsComponentData>&)>& onFound) const
{
    auto foundIt = m_columnFlagsComponents.find(column);
    if(foundIt != m_columnFlagsComponents.end()) {
        const auto& columnsHandler = foundIt.value();
        onFound(columnsHandler);
        return true;
    }
    foundIt = m_columnFlagsComponents.find(-1);
    if(foundIt != m_columnFlagsComponents.end()) {
        const auto& columnsHandler = foundIt.value();
        onFound(columnsHandler);
        return true;
    }
    return false;
}
