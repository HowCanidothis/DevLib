#include "modelstablebase.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

ViewModelsTableBase::ViewModelsTableBase(QObject* parent)
    : Super(parent)
{

}

ViewModelsTableBase::~ViewModelsTableBase()
{
    if(m_data != nullptr) {
        m_data->DisconnectModel(this);
    }
}

Qt::ItemFlags ViewModelsTableBase::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags result;
    if(ColumnComponents.GetFlags(index, result)) {
        return result;
    }
    return Super::flags(index);
}

QVariant ViewModelsTableBase::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    QVariant result;
    if(ColumnComponents.GetData(index, role, result)) {
        return result;
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
    bool result;
    if(ColumnComponents.SetData(index, data, role, result)) {
        return result;
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
        QVariant result;
        if(ColumnComponents.GetHeaderData(section, role, result)) {
            return result;
        }

        auto foundIt = m_roleHorizontalHeaderDataHandlers.find(role);
        if(foundIt == m_roleHorizontalHeaderDataHandlers.end()) {
            return QVariant();
        }
        return foundIt.value()(section);
    } else {
        auto foundIt = m_roleVerticalHeaderDataHandlers.find(role);
        if(foundIt == m_roleVerticalHeaderDataHandlers.end()) {
            return QVariant();
        }
        return foundIt.value()(section);
    }
}

void ViewModelsTableBase::SetData(const ModelsTableWrapperPtr& data)
{
    if(m_data == data) {
        return;
    }

    beginResetModel();
    if(m_data != nullptr) {
        m_data->DisconnectModel(this);
    }
    m_data = data;
    if(m_data != nullptr) {
        m_data->ConnectModel(this);
    }
    endResetModel();

    OnModelChanged();
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

bool ViewModelsTableColumnComponents::SetData(const QModelIndex& index, const QVariant& data, qint32 role, bool& result)
{
    bool accept = true;
    auto testHandler = [&](const QVector<ColumnComponentData>& components){
        for(const auto& handlers : components) {
            accept = true;
            result = handlers.SetterHandler(index, data, accept);
            if(accept) {
                break;
            }
        }
    };

    if(callHandler(index.column(), (Qt::ItemDataRole)role, testHandler)) {
        return accept;
    }
    return false;
}

bool ViewModelsTableColumnComponents::GetData(const QModelIndex& index, qint32 role, QVariant& data) const
{
    bool accept = true;
    auto testHandler = [&](const QVector<ColumnComponentData>& components){
        for(const auto& handlers : components) {
            accept = true;
            data = handlers.GetterHandler(index, accept);
            if(accept) {
                break;
            }            
        }
    };

    if(callHandler(index.column(), (Qt::ItemDataRole)role, testHandler)) {
        return accept;
    }
    return false;
}

bool ViewModelsTableColumnComponents::GetHeaderData(qint32 section, qint32 role, QVariant& header) const
{
    bool accept = true;
    auto testHandler = [&](const QVector<ColumnComponentData>& data){
        for(const auto& handlers : data) {
            accept = true;
            header = handlers.GetHeaderHandler(accept);
            if(accept) {
                break;
            }            
        }
    };

    if(callHandler(section, (Qt::ItemDataRole)role, testHandler)) {
        return accept;
    }
    return false;
}

bool ViewModelsTableColumnComponents::GetFlags(const QModelIndex& index, Qt::ItemFlags& flags) const
{
    bool accept = true;
    auto testHandler = [&](const QVector<ColumnFlagsComponentData>& data){
        for(const auto& handlers : data) {
            accept = true;
            flags = handlers.GetFlagsHandler(index.row(), accept);
            if(accept) {
                break;
            }
        }
    };

    if(callFlagsHandler(index.column(), testHandler)) {
        return accept;
    }
    return false;
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
