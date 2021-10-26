#ifndef MODELSFILTERMODELBASE_H
#define MODELSFILTERMODELBASE_H

#include <QSortFilterProxyModel>

#include <PropertiesModule/internal.hpp>

class ModelsFilterModelBase : public QSortFilterProxyModel
{
    using Super = QSortFilterProxyModel;
public:
    ModelsFilterModelBase(QObject* parent);

    void InvalidateFilter();
    bool IsLastEditRow(const QModelIndex& index) const
    {
        return (sourceModel()->rowCount() - 1) == index.row();
    }
    bool DefaultLessThan(const QModelIndex& f, const QModelIndex& s) const
    {
        return Super::lessThan(f,s);
    }

    bool filterAcceptsRow(qint32 sourceRow, const QModelIndex& sourceParent) const override;

    std::function<bool (qint32, const QModelIndex&)> FilterHandler;
    std::function<bool (const QModelIndex&, const QModelIndex&)> LessThan;
    Dispatcher OnInvalidated;

protected:
    bool lessThan(const QModelIndex& f, const QModelIndex& s) const override;

private:
    DelayedCallObject m_invalidateFilter;
};

struct ModelsTextFilterData
{
    LocalPropertyString FilterString;

    bool HasFilter() const { return !FilterString.Native().isEmpty(); }

    Dispatcher OnChange;

    ModelsTextFilterData()
    {
        OnChange.ConnectFrom(FilterString.OnChange);
    }
};

template<class FilterClassData>
class ModelsTreeFilterComponent
{
public:
    using FFilterHandler = std::function<bool (const FilterClassData& filterData, const class ModelsTreeItemBase*)>;

    ModelsTreeFilterComponent(ModelsFilterModelBase* proxy, const FFilterHandler& handler)
        : m_sourceModel(qobject_cast<ViewModelTreeBase*>(proxy->sourceModel()))
        , m_filterHandler(handler)
        , m_proxyModel(proxy)
    {
        Q_ASSERT(m_sourceModel != nullptr);

        proxy->FilterHandler = [this](qint32 sourceRow, const QModelIndex& sourceParent) {
            return filterAcceptsRow(sourceRow, sourceParent);
        };

        FilterData.OnChange += { this, [this, proxy]{
            proxy->InvalidateFilter();
        }};

        QObject::connect(proxy, &QSortFilterProxyModel::destroyed, [this]{ delete this; });
    }

    FilterClassData FilterData;

    void ConnectTreeView(QTreeView* treeView)
    {
        Q_ASSERT(treeView->model() == m_proxyModel && m_proxyModel->sourceModel() == m_sourceModel);
        m_qtConnections.connect(treeView, &QTreeView::expanded, [treeView, this](const QModelIndex& index){
            auto* item = m_sourceModel->AsItem(m_proxyModel->mapToSource(index));
            if(item != nullptr) {
                item->SetExpanded((size_t)treeView, true);
            }
        });
        m_qtConnections.connect(treeView, &QTreeView::collapsed, [treeView, this](const QModelIndex& index){
            auto* item = m_sourceModel->AsItem(m_proxyModel->mapToSource(index));
            if(item != nullptr) {
                item->SetExpanded((size_t)treeView, false);
            }
        });
        m_proxyModel->OnInvalidated.Connect(this, [this, treeView]{
            forEachModelIndex(m_proxyModel, QModelIndex(), [this,treeView](const QModelIndex& index){
                auto* item = m_sourceModel->AsItem(m_proxyModel->mapToSource(index));
                if(item != nullptr && item->IsExpanded((size_t)treeView)) {
                    treeView->expand(index);
                }
                return true;
            });
        }).MakeSafe(m_connections);
    }

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        if(!FilterData.HasFilter()) {
            return true;
        }

        auto* parent = m_sourceModel->AsItem(source_parent);
        auto item = parent->GetChilds()[source_row];

        bool isMatch = m_filterHandler(FilterData, item.get());
        auto matchHandler = [this, &isMatch](const ModelsTreeItemBase* item){
            if(m_filterHandler(FilterData, item)){
                isMatch = true;
            }
        };
        auto skipHandler = [&isMatch](const ModelsTreeItemBase* ){ return !isMatch; };

        item->ForeachParent(matchHandler, skipHandler);
        item->ForeachChild(matchHandler, skipHandler);
        return isMatch;
    }

private:
    class ViewModelTreeBase* m_sourceModel;
    FFilterHandler m_filterHandler;
    QtLambdaConnections m_qtConnections;
    ModelsFilterModelBase* m_proxyModel;
    DispatcherConnectionsSafe m_connections;
};

template<class T>
class TModelsTableBase;

template<class FilterClassData, class Wrapper>
class ModelsTableFilterComponent
{
public:
    using value_type = typename Wrapper::value_type;
    using FFilterHandler = std::function<bool (const FilterClassData& filterData, qint32 index)>;

    ModelsTableFilterComponent(ModelsFilterModelBase* proxy, const FFilterHandler& handler)
        : m_sourceModel(reinterpret_cast<TModelsTableBase<Wrapper>*>(proxy->sourceModel()))
        , m_filterHandler(handler)
        , m_proxyModel(proxy)
    {
        Q_ASSERT(m_sourceModel != nullptr);

        proxy->FilterHandler = [this](qint32 sourceRow, const QModelIndex&) {
            if(!FilterData.HasFilter()) {
                return true;
            }
            return m_filterHandler(FilterData, sourceRow);
        };

        FilterData.OnChange += { this, [this, proxy]{
            proxy->InvalidateFilter();
        }};

        QObject::connect(proxy, &QSortFilterProxyModel::destroyed, [this]{ delete this; });
    }

    FilterClassData FilterData;

private:
    TModelsTableBase<Wrapper>* m_sourceModel;
    FFilterHandler m_filterHandler;
    QtLambdaConnections m_qtConnections;
    ModelsFilterModelBase* m_proxyModel;
    DispatcherConnectionsSafe m_connections;
};


#endif // MODELSFILTERMODELBASE_H
