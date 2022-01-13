#include "modelsfiltermodelbase.h"

ViewModelsFilterModelBase::ViewModelsFilterModelBase(QObject* parent)
    : Super(parent)
    , FilterColumnHandler([](qint32, const QModelIndex&){ return true; })
    , FilterHandler([](qint32, const QModelIndex&){ return true; })
    , LessThan([this](const QModelIndex& f, const QModelIndex& s){ return Super::lessThan(f,s); })
    , m_invalidateFilter(500)
{}

void ViewModelsFilterModelBase::InvalidateFilter()
{
    m_invalidateFilter.Call([this]{
        invalidateFilter();
        OnInvalidated();
    });
}

QVariant ViewModelsFilterModelBase::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return section + 1;
    }
    return Super::headerData(section, orientation, role);
}

bool ViewModelsFilterModelBase::filterAcceptsColumn(qint32 sourceColumn, const QModelIndex& sourceParent) const
{
    return FilterColumnHandler(sourceColumn, sourceParent);
}

bool ViewModelsFilterModelBase::filterAcceptsRow(qint32 sourceRow, const QModelIndex& sourceParent) const
{
    return FilterHandler(sourceRow, sourceParent);
}

bool ViewModelsFilterModelBase::lessThan(const QModelIndex& f, const QModelIndex& s) const
{
    return LessThan(f, s);
}
