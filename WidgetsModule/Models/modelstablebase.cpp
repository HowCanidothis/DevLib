#include "modelstablebase.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

void ModelsWrapperBase::ConnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);

    OnAboutToBeReset += { model, [this, model]{
        if(m_resetViewModelOnReset) {
            model->beginResetModel();
        } else {
            emit model->layoutAboutToBeChanged();
        }
    }};
    OnReset += { model, [this, model]{
        if(m_resetViewModelOnReset) {
            model->endResetModel();
        } else {
            emit model->layoutChanged();
        }
    }};
    OnAboutToBeUpdated += { model, [model]{
        emit model->layoutAboutToBeChanged();
    }};
    OnUpdated += { model, [model]{
        emit model->layoutChanged();
    }};
    OnRowsRemoved += { model, [model]{ model->endRemoveRows(); } };
    OnRowsInserted += { model, [model](qint32, qint32){ model->endInsertRows(); }};
    OnRowsChanged += { model, [model] (qint32 row, qint32 count, const QSet<qint32>& columns){
        auto startmi = model->index(row, columns.isEmpty() ? 0 : *columns.begin());
//        Q_ASSERT(columns.isEmpty() || *(columns.end()-1) == *std::max_element(columns.constBegin(), columns.constEnd()));//ебанет?
        auto endmi = model->index(row + count - 1, columns.isEmpty() ? model->columnCount() : *std::max_element(columns.constBegin(), columns.constEnd()));
        model->dataChanged(startmi, endmi);
    }};
}

void ModelsWrapperBase::DisconnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);

    OnAboutToBeReset -= model;
    OnReset -= model;
    OnAboutToBeUpdated -= model;
    OnUpdated -= model;
    OnRowsRemoved -= model;
    OnRowsInserted -= model;
    OnAboutToBeDestroyed -= model;
    OnRowsChanged -= model;
}

void ModelsTreeWrapper::ConnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);
    Super::ConnectModel(qmodel);
    OnAboutToInsertRows += { model, [model](qint32 start,qint32 end, ModelsTreeItemBase* parent){
        if(parent->GetParent() == nullptr) {
            model->beginInsertRows(QModelIndex(), start, end);
        } else {
            model->beginInsertRows(model->createIndex(parent->GetRow(), 0, parent), start, end);
        }
    }};
    OnAboutToRemoveRows += { model, [model](qint32 start,qint32 end, ModelsTreeItemBase* parent){
        if(parent->GetParent() == nullptr) {
            model->beginRemoveRows(QModelIndex(), start, end);
        } else {
            model->beginRemoveRows(model->createIndex(parent->GetRow(), 0, parent), start, end);
        }
    }};
    OnTreeValueChanged += {model, [model](size_t, ModelsTreeItemBase* item, QVector<int> roles){
        auto index = model->createIndex(item->GetRow(), 0, item);
        emit model->dataChanged(index, index, roles);
    }};
}

void ModelsTreeWrapper::DisconnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);
    Super::DisconnectModel(qmodel);
    OnAboutToRemoveRows -= model;
    OnAboutToInsertRows -= model;
    OnTreeValueChanged -= model;
}

void ModelsTableWrapper::ConnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);
    Super::ConnectModel(qmodel);
    OnValueChanged += { model, [model](qint32 row, qint32 column) {
        auto modelIndex = model->index(row, column);
        emit model->dataChanged(modelIndex, modelIndex);
    }};
    OnAboutToRemoveRows += { model, [model](qint32 start,qint32 end){ model->beginRemoveRows(QModelIndex(), start, end); } };
    OnAboutToInsertRows += { model, [model](qint32 start,qint32 end){ model->beginInsertRows(QModelIndex(), start, end); } };
}

void ModelsTableWrapper::DisconnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);
    Super::DisconnectModel(qmodel);
    OnAboutToRemoveRows -= model;
    OnAboutToInsertRows -= model;
    OnValueChanged -= model;
}

ViewModelsTableBase::ViewModelsTableBase(QObject* parent)
    : Super(parent)
    , IsEditable(true)
    , m_mostLeftColumnToUpdate(-1)
    , m_mostRightColumnToUpdate(-1)
{

}

ViewModelsTableBase::~ViewModelsTableBase()
{

}

bool ViewModelsTableBase::IsLastRow(const QModelIndex& index) const
{
    return index.data(LastEditRowRole).toBool();
}

bool ViewModelsTableBase::IsLastRow(qint32 row) const
{
    return IsLastRow(index(row, 0));
}

qint32 ViewModelsTableBase::columnCount(const QModelIndex&) const
{
    return ColumnComponents.GetColumnCount();
}

void ViewModelsTableBase::RequestUpdateUi(const char* locationInfo, qint32 left, qint32 right)
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
    m_update.Call(locationInfo, [this]{
        emit dataChanged(createIndex(0, m_mostLeftColumnToUpdate), createIndex(rowCount()-1, m_mostRightColumnToUpdate));
        emit headerDataChanged(Qt::Horizontal, m_mostLeftColumnToUpdate, m_mostRightColumnToUpdate);
        m_mostLeftColumnToUpdate = -1;
        m_mostRightColumnToUpdate = -1;
    });
}

#ifdef UNITS_MODULE_LIB
DispatcherConnection ViewModelsTableBase::AttachTempDependence(const char* locationInfo, const Measurement* unitName, int first, int last)
{
    return AttachTempDependence(locationInfo, &unitName->OnChanged, first, last);
}

void ViewModelsTableBase::AttachDependence(const char* locationInfo, const Measurement* unitName, int first, int last)
{
    AttachDependence(locationInfo, &unitName->OnChanged, first, last);
}
#endif

DispatcherConnection ViewModelsTableBase::AttachTempDependence(const char* locationInfo, const Dispatcher* dispatcher, int first, int last)
{
    return dispatcher->Connect(locationInfo, [locationInfo, first, last, this]{
        RequestUpdateUi(locationInfo, first, last);
    });
}

void ViewModelsTableBase::AttachDependence(const char* locationInfo, const Dispatcher* dispatcher, int first, int last)
{
    AttachTempDependence(locationInfo, dispatcher, first, last).MakeSafe(m_connections);
}

Qt::ItemFlags ViewModelsTableBase::flags(const QModelIndex& index) const
{
    if(!IsEnabled()) {
        return StandardNonEditableFlags();
    }
    if(!index.isValid()) {
        return Qt::ItemIsDropEnabled;
    }
    auto componentsResult = ColumnComponents.GetFlags(index);
    if(componentsResult.has_value()) {
        return componentsResult.value() | Qt::ItemIsDragEnabled;
    }

    return StandardNonEditableFlags();
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
    if(!IsEnabled()) {
        return false;
    }

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
            if(role == Qt::TextAlignmentRole) {
                return qVariantFromValue(Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap);
            }
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

bool ViewModelsTableColumnComponents::SetComponent(qint32 role /*Qt::ItemDataRole*/, qint32 column, qint32 index, const ColumnComponentData& columnData)
{
    auto foundIt = m_columnComponents.find(column);
    if(foundIt == m_columnComponents.end()) {
        return false;
    }
    auto& components = foundIt.value()[role];
    if(components.size() <= index) {
        return false;
    }
    components[index] = columnData;
    return true;
}

qint32 ViewModelsTableColumnComponents::AddComponent(qint32 role, qint32 column, const ColumnComponentData& columnData)
{
    auto foundIt = m_columnComponents.find(column);
    if(foundIt == m_columnComponents.end()) {
        foundIt = m_columnComponents.insert(column, {});
    }
    auto& components = foundIt.value()[role];
    components.append(columnData);
    return components.size() - 1;
}

void ViewModelsTableColumnComponents::AddFlagsComponent(qint32 column, const ColumnFlagsComponentData& flagsColumnData)
{
    auto foundIt = m_columnFlagsComponents.find(column);
    if(foundIt == m_columnFlagsComponents.end()) {
        foundIt = m_columnFlagsComponents.insert(column, {});
    }
    foundIt.value().append(flagsColumnData);
}

void ViewModelsTableColumnComponents::AddFlagsComponent(qint32 column, const ColumnFlagsComponentData::FHandler& handler)
{
    AddFlagsComponent(column, ColumnFlagsComponentData(handler));
}

void ViewModelsTableColumnComponents::AddFlagsComponent(const QVector<qint32>& columns, const ColumnFlagsComponentData::FHandler& handler)
{
    for(auto column : columns) {
        AddFlagsComponent(column, ColumnFlagsComponentData(handler));
    }
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

bool ViewModelsTableColumnComponents::callHandler(qint32 column, qint32 role, const std::function<void (const QVector<ColumnComponentData> &)> &onFound) const
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

ViewModelsTableColumnComponents::ColumnFlagsComponentData::ColumnFlagsComponentData(const FHandler& handler)
    : GetFlagsHandler(handler)
{

}

template<>
qint32 ViewModelsVerticalCompoundTable::totalSizeOf<adapters::Range<QVector<qint32*>::const_iterator>>(const adapters::Range<QVector<qint32*>::const_iterator>& range) const
{
    qint32 result(0);
    for(auto count : range) {
        result += *count;
    }
    return result;
}


ViewModelsVerticalCompoundTable::ViewModelsVerticalCompoundTable(QObject* parent)
    : Super(parent)
{}

void ViewModelsVerticalCompoundTable::SetModels(const QVector<ViewModelsTableBase*>& models)
{
    m_connections.Clear();
    beginResetModel();

    m_models = models;
    m_modelsRows.resize(models.size());
    qint32 modelIndex = 0;
    QVector<qint32*> currentRowsOffsets;
    for(ViewModelsTableBase* model : models) {
        currentRowsOffsets.append(&m_modelsRows[modelIndex]);
        *currentRowsOffsets.last() = model->rowCount();
        auto watcher = ::make_shared<QVector<qint32*>>(currentRowsOffsets);
        m_connections.connect(model, &QAbstractTableModel::layoutAboutToBeChanged, this, [this]{
            emit layoutAboutToBeChanged();
        });
        m_connections.connect(model, &QAbstractTableModel::layoutChanged, this, [this]{
            emit layoutChanged();
        });
        m_connections.connect(model, &QAbstractTableModel::rowsAboutToBeInserted, this, [this, watcher](const QModelIndex&, int first, int last){
            auto sizeBefore = totalSizeOf(adapters::withoutLast(::make_const(*watcher)));
            beginInsertRows(QModelIndex(), sizeBefore + first, sizeBefore + last);
        });
        m_connections.connect(model, &QAbstractTableModel::rowsInserted, this, [this, watcher](const QModelIndex&, int first, int last){
            auto inserted = last - first + 1;
            *watcher->last() += inserted;
            m_rowsCount += inserted;
            endInsertRows();
        });
        m_connections.connect(model, &QAbstractTableModel::rowsAboutToBeRemoved, this, [this, watcher](const QModelIndex&, int first, int last){
            auto sizeBefore = totalSizeOf(adapters::withoutLast(::make_const(*watcher)));
            beginRemoveRows(QModelIndex(), sizeBefore + first, sizeBefore + last);
        });
        m_connections.connect(model, &QAbstractTableModel::rowsRemoved, this, [this](const QModelIndex&, int first, int last){
            m_rowsCount -= last - first + 1;
            endRemoveRows();
        });
        m_connections.connect(model, &QAbstractTableModel::modelAboutToBeReset, this, [this]{
            beginResetModel();
        });
        m_connections.connect(model, &QAbstractTableModel::modelReset, this, [this, model, watcher]{
            *watcher->last() = model->rowCount();
            m_rowsCount = totalSizeOf(m_modelsRows);
            endResetModel();
        });
        ++modelIndex;
    }

    m_rowsCount = totalSizeOf(m_modelsRows);

    endResetModel();
}

Qt::ItemFlags ViewModelsVerticalCompoundTable::flags(const QModelIndex& index) const
{
    Qt::ItemFlags result;
    indexToSourceIndex(index, [&](const QModelIndex& i, ViewModelsTableBase* model) {
        result = model->flags(i);
    });
    return result;
}

bool ViewModelsVerticalCompoundTable::setData(const QModelIndex& index, const QVariant& data, qint32 role)
{
    bool result;
    indexToSourceIndex(index, [&](const QModelIndex& i, ViewModelsTableBase* model) {
        result = model->setData(i, data, role);
    });
    return result;
}

qint32 ViewModelsVerticalCompoundTable::columnCount(const QModelIndex& parent) const
{
    if(m_models.isEmpty()) {
        return 0;
    }
    return m_models.first()->columnCount(parent);
}

qint32 ViewModelsVerticalCompoundTable::rowCount(const QModelIndex& index) const
{
    return m_rowsCount;
}

bool ViewModelsVerticalCompoundTable::insertRows(int row, int count, const QModelIndex&)
{
    if(row < 0 || count <= 0) {
        return false;
    }

    qint32 anchor;
    qint32 rowsCounter = 0;
    adapters::Foreach([&](qint32 rows, ViewModelsTableBase* viewModel) {
        if(count == 0) {
            return;
        }
        rowsCounter += rows;
        if(rowsCounter > row) {
            anchor = row - rowsCounter + rows;
            viewModel->insertRows(anchor, count);
            count = 0;
        }
    }, m_modelsRows, m_models);
}

bool ViewModelsVerticalCompoundTable::removeRows(int row, int count, const QModelIndex&)
{
    if(count == 0) {
        return false;
    }
    qint32 anchor;
    qint32 rowsCounter = 0;
    adapters::Foreach([&](qint32 rows, ViewModelsTableBase* viewModel) {
        if(count == 0) {
            return;
        }
        rowsCounter += rows;
        if(rowsCounter > row) {
            anchor = std::max(row - rowsCounter + rows, 0);
            auto maxToRemove = rows - anchor;
            auto delta = count - maxToRemove;
            if(delta <= 0) {
                viewModel->removeRows(anchor, count);
                count = 0;
            } else {
                viewModel->removeRows(anchor, maxToRemove);
                count = delta;
            }
        }
    }, m_modelsRows, m_models);
    return count == 0;
}

QVariant ViewModelsVerticalCompoundTable::data(const QModelIndex& index, qint32 role) const
{
    QVariant result;
    indexToSourceIndex(index, [&](const QModelIndex& i, ViewModelsTableBase*) {
        result = i.data(role);
    });
    return result;
}

QVariant ViewModelsVerticalCompoundTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(m_models.isEmpty()) {
        return QVariant();
    }
    return m_models.first()->headerData(section, orientation, role);
}

QStringList ViewModelsVerticalCompoundTable::mimeTypes() const
{
    return QStringList(); // TODO
}

QMimeData* ViewModelsVerticalCompoundTable::mimeData(const QModelIndexList& indices) const
{
    return nullptr; // TODO
}

bool ViewModelsVerticalCompoundTable::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    return false; // TODO
}

bool ViewModelsVerticalCompoundTable::dropMimeData(const QMimeData* data, Qt::DropAction action, qint32 row, qint32 column, const QModelIndex& parent)
{
    return false; // TODO
}

bool ViewModelsVerticalCompoundTable::indexToSourceIndex(const QModelIndex& modelIndex, const std::function<void (const QModelIndex&, ViewModelsTableBase*)>& handler) const
{
    if(!modelIndex.isValid()) {
        return false;
    }

    auto row = modelIndex.row();
    qint32 resultRow = -1;
    qint32 rowsCounter = 0;
    ViewModelsTableBase* model = nullptr;
    adapters::Foreach([&](qint32 rows, ViewModelsTableBase* viewModel) {
        if(model != nullptr) {
            return;
        }
        rowsCounter += rows;
        if(rowsCounter > row) {
            resultRow = row - rowsCounter + rows;
            model = viewModel;
        }
    }, m_modelsRows, m_models);

    if(model != nullptr) {
        handler(model->index(resultRow, modelIndex.column()), model);
        return true;
    }
    return false;
}
