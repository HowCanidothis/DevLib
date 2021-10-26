#include "modelsfiltermodelbase.h"

ModelsFilterModelBase::ModelsFilterModelBase(QObject* parent)
    : Super(parent)
    , FilterColumnHandler([](qint32, const QModelIndex&){ return true; })
    , FilterHandler([](qint32, const QModelIndex&){ return true; })
    , LessThan([this](const QModelIndex& f, const QModelIndex& s){ return Super::lessThan(f,s); })
    , m_invalidateFilter(500)
{}

void ModelsFilterModelBase::InvalidateFilter()
{
    m_invalidateFilter.Call([this]{
        invalidateFilter();
        OnInvalidated();
    });
}

bool ModelsFilterModelBase::filterAcceptsColumn(qint32 sourceColumn, const QModelIndex& sourceParent) const
{
    return FilterColumnHandler(sourceColumn, sourceParent);
}

bool ModelsFilterModelBase::filterAcceptsRow(qint32 sourceRow, const QModelIndex& sourceParent) const
{
    return FilterHandler(sourceRow, sourceParent);
}

bool ModelsFilterModelBase::lessThan(const QModelIndex& f, const QModelIndex& s) const
{
    return LessThan(f, s);
}
