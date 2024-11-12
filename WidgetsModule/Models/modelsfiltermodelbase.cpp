#include "modelsfiltermodelbase.h"

ViewModelsFilterModelBase::ViewModelsFilterModelBase(QObject* parent)
    : Super(parent)
    , SetDataHandler([this](const QModelIndex& index, const QVariant& data, qint32 role) -> bool { return Super::setData(index, data, role); })
    , GetFlagsHandler([this](const QModelIndex& index) { return Super::flags(index); })
    , GetDataHandler([this](const QModelIndex& index, qint32 role){ return Super::data(index, role); })
    , FilterColumnHandler([](qint32, const QModelIndex&){ return true; })
    , FilterHandler([](qint32, const QModelIndex&){ return true; })
    , LessThan([this](const QModelIndex& f, const QModelIndex& s){ return Super::lessThan(f,s); })
    , CanDropMimeDataHandler(DefaultCanDropMimeDataHandler())
    , DropMimeDataHandler(DefaultDropMimeDataHandler())
    , m_invalidateFilter(500)
{
    UseSourceEnumeration.Connect(CDL, [this](bool) {
        auto rc = rowCount();
        if(rc) {
            emit headerDataChanged(Qt::Vertical, 0, rc - 1);
        }
    });

    Invalidate.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        InvalidateFilter();
    });

    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void ViewModelsFilterModelBase::InvalidateFilter()
{
    m_invalidateFilter.Call(CONNECTION_DEBUG_LOCATION, [this]{
        invalidateFilter();
        OnInvalidated();
    });
}

bool ViewModelsFilterModelBase::IsLastRow(const QModelIndex& index) const
{
    return index.data(LastEditRowRole).toBool();
}

bool ViewModelsFilterModelBase::IsLastRow(qint32 row) const
{
    return IsLastRow(index(row, 0));
}

bool ViewModelsFilterModelBase::IsLastSourceRow(qint32 sourceRow) const
{
    if(sourceModel() == nullptr) {
        return false;
    }
    return sourceModel()->index(sourceRow, 0).data(LastEditRowRole).toBool();
}

QModelIndex ViewModelsFilterModelBase::SourceModelIndex(qint32 sourceRow, qint32 sourceColumn, const QModelIndex& sourceParent) const
{
    if(sourceModel() == nullptr) {
        return QModelIndex();
    }
    return sourceModel()->index(sourceRow, sourceColumn, sourceParent);
}

bool ViewModelsFilterModelBase::DefaultLessThan(const QModelIndex& f, const QModelIndex& s) const
{
    return Super::lessThan(f,s);
}

bool ViewModelsFilterModelBase::DefaultCanDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    return Super::canDropMimeData(data, action, row, column, parent);
}

bool ViewModelsFilterModelBase::DefaultDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    return Super::dropMimeData(data, action, row, column, parent);
}

void ViewModelsFilterModelBase::setSourceModel(QAbstractItemModel* m)
{
    auto* oldModel = sourceModel();
    Super::setSourceModel(m);
    if(oldModel != m) {
        OnModelChanged();
    }
}

QVariant ViewModelsFilterModelBase::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    return GetDataHandler(index, role);
}

QVariant ViewModelsFilterModelBase::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const
{
    if(UseSourceEnumeration) {
        return Super::headerData(section, orientation, role);
    }

    if(role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return section + 1;
    }
    return Super::headerData(section, orientation, role);
}

Qt::ItemFlags ViewModelsFilterModelBase::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags | Qt::ItemIsDropEnabled;
    }

    return GetFlagsHandler(index);
}

void ViewModelsFilterModelBase::SetColumnFilter(const std::function<bool (qint32, const QModelIndex&)>& handler){
    FilterColumnHandler = handler;
    InvalidateFilter();
}

void ViewModelsFilterModelBase::SetRowFilter(const std::function<bool (qint32, const QModelIndex&)>& handler){
    FilterHandler = handler;
    InvalidateFilter();
}

bool ViewModelsFilterModelBase::filterAcceptsColumn(qint32 sourceColumn, const QModelIndex& sourceParent) const
{
    return FilterColumnHandler(sourceColumn, sourceParent);
}

bool ViewModelsFilterModelBase::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    return CanDropMimeDataHandler(data, action, row, column, parent);
}

bool ViewModelsFilterModelBase::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    return DropMimeDataHandler(data, action, row, column, parent);
}

bool ViewModelsFilterModelBase::setData(const QModelIndex& index, const QVariant& data, qint32 role)
{
    return SetDataHandler(index, data, role);
}

bool ViewModelsFilterModelBase::filterAcceptsRow(qint32 sourceRow, const QModelIndex& sourceParent) const
{
    return FilterHandler(sourceRow, sourceParent);
}

bool ViewModelsFilterModelBase::lessThan(const QModelIndex& f, const QModelIndex& s) const
{
    return LessThan(f, s);
}
