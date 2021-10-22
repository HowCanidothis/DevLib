#include "modelsfiltermodelbase.h"

ModelsFilterModelBase::ModelsFilterModelBase(QObject* parent)
    : Super(parent)
    , FilterHandler([](qint32, const QModelIndex&){ return true; })
    , m_invalidateFilter(500)
{}

void ModelsFilterModelBase::InvalidateFilter()
{
    m_invalidateFilter.Call([this]{
        invalidateFilter();
        OnInvalidated();
    });
}

bool ModelsFilterModelBase::filterAcceptsRow(qint32 sourceRow, const QModelIndex& sourceParent) const
{
    return FilterHandler(sourceRow, sourceParent);
}
