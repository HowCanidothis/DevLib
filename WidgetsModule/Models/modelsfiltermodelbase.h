#ifndef MODELSFILTERMODELBASE_H
#define MODELSFILTERMODELBASE_H

#include <QSortFilterProxyModel>

#include <PropertiesModule/internal.hpp>

class ViewModelsFilterModelBase : public QSortFilterProxyModel
{
public:
    using Super = QSortFilterProxyModel;
    ViewModelsFilterModelBase(QObject* parent);

    void InvalidateFilter();
    bool IsLastEditRow(const QModelIndex& index) const
    {
        return IsLastEditRow(index.row());
    }
    bool IsLastEditRow(qint32 row) const
    {
        return (sourceModel()->rowCount() - 1) == row;
    }
    bool DefaultLessThan(const QModelIndex& f, const QModelIndex& s) const
    {
        return Super::lessThan(f,s);
    }

    void setSourceModel(QAbstractItemModel* m) override;

    bool filterAcceptsRow(qint32 sourceRow, const QModelIndex& sourceParent) const override;
    bool filterAcceptsColumn(qint32 sourceColumn, const QModelIndex& sourceParent) const override;

    bool setData(const QModelIndex& index, const QVariant& data, qint32 role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, qint32 role) const override;
    QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role) const override;

    void SetRowFilter(const std::function<bool (qint32,const QModelIndex&)>& handler);
    void SetColumnFilter(const std::function<bool (qint32, const QModelIndex&)>& handler);

    std::function<bool (const QModelIndex&, const QVariant&, qint32 role)> SetDataHandler;
    std::function<Qt::ItemFlags (const QModelIndex&)> GetFlagsHandler;
    std::function<QVariant (const QModelIndex&, qint32)> GetDataHandler;
    std::function<bool (qint32, const QModelIndex&)> FilterColumnHandler;
    std::function<bool (qint32, const QModelIndex&)> FilterHandler;
    std::function<bool (const QModelIndex&, const QModelIndex&)> LessThan;
    Dispatcher OnInvalidated;
    Dispatcher Invalidate;
    Dispatcher OnModelChanged;

protected:
    bool lessThan(const QModelIndex& f, const QModelIndex& s) const override;

private:
    DelayedCallObject m_invalidateFilter;
};

struct ViewModelsTextFilterData
{
    LocalPropertyString FilterString;

    bool HasFilter() const { return !FilterString.Native().isEmpty(); }

    Dispatcher OnChanged;

    ViewModelsTextFilterData()
    {
        OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, FilterString.OnChanged);
    }
};

template<class FilterClassData>
class ViewModelsTreeFilterComponent
{
public:
    using FFilterHandler = std::function<bool (const FilterClassData& filterData, const class ModelsTreeItemBase*)>;

    ViewModelsTreeFilterComponent(ViewModelsFilterModelBase* proxy, const FFilterHandler& handler)
        : m_sourceModel(qobject_cast<ViewModelsTreeBase*>(proxy->sourceModel()))
        , m_filterHandler(handler)
        , m_proxyModel(proxy)
    {
        Q_ASSERT(m_sourceModel != nullptr);

        proxy->FilterHandler = [this](qint32 sourceRow, const QModelIndex& sourceParent) {
            return filterAcceptsRow(sourceRow, sourceParent);
        };

        FilterData.OnChanged += { this, [this, proxy]{
            proxy->InvalidateFilter();
        }};

        QObject::connect(proxy, &QSortFilterProxyModel::destroyed, [this]{ delete this; });
    }

    FilterClassData FilterData;

    void ConnectTreeView(class QTreeView* treeView)
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
        m_proxyModel->OnInvalidated.Connect(CONNECTION_DEBUG_LOCATION, [this, treeView]{
            ViewModelWrapper(m_proxyModel).ForeachModelIndex([this,treeView](const QModelIndex& index){
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
    class ViewModelsTreeBase* m_sourceModel;
    FFilterHandler m_filterHandler;
    QtLambdaConnections m_qtConnections;
    ViewModelsFilterModelBase* m_proxyModel;
    DispatcherConnectionsSafe m_connections;
};

template<class T>
class TViewModelsTableBase;

template<class FilterClassData, class Wrapper>
class ViewModelsTableFilterComponent : public QObject
{
public:
    using value_type = typename Wrapper::value_type;
    using FFilterHandler = std::function<bool (const FilterClassData& filterData, qint32 index)>;

    ViewModelsTableFilterComponent(ViewModelsFilterModelBase* proxy, const FFilterHandler& handler)
        : QObject(proxy)
        , m_sourceModel(reinterpret_cast<TViewModelsTableBase<Wrapper>*>(proxy->sourceModel()))
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

        FilterData.OnChanged += { this, [this, proxy]{
            proxy->InvalidateFilter();
        }};

        QObject::connect(proxy, &QSortFilterProxyModel::destroyed, [this]{ delete this; });
    }

    FilterClassData FilterData;

private:
    TViewModelsTableBase<Wrapper>* m_sourceModel;
    FFilterHandler m_filterHandler;
    QtLambdaConnections m_qtConnections;
    ViewModelsFilterModelBase* m_proxyModel;
    DispatcherConnectionsSafe m_connections;
};

template<typename Selection>
class ViewModelsSelectionFilterModel : public ViewModelsFilterModelBase
{
    using Super = ViewModelsFilterModelBase;
    using FExtractor = std::function<Selection (const QModelIndex& index)>;
public:
    ViewModelsSelectionFilterModel(QObject* parent, const FExtractor& extractor)
        : Super(parent)
        , m_extractor(extractor)
    {}

    LocalPropertySet<Selection> Selected;

    void ClearSelection()
    {
        Selected.Clear();
    }

    void Deselect(const QModelIndexList& indices)
    {
        bool changed = false;
        auto& silent = Selected.EditSilent();
        for(const auto& index : indices) {
            if(index.column() == 0) {
                silent.remove(m_extractor(mapToSource(index)));
                changed = true;
            }
        }
        if(changed) {
            Selected.Invoke();
        }
    }

    void Select(const QModelIndexList& indices)
    {
        bool changed = false;
        auto& silent = Selected.EditSilent();
        for(const auto& index : indices) {
            if(index.column() == 0) {
                silent.insert(m_extractor(mapToSource(index)));
                changed = true;
            }
        }
        if(changed) {
            Selected.Invoke();
        }
    }

    bool lessThan(const QModelIndex& f, const QModelIndex& s) const override
    {
        bool fContains = Selected.IsContains(m_extractor(f));
        bool sContains = Selected.IsContains(m_extractor(s));
        if(fContains != sContains) {
            return fContains;
        }
        return Super::lessThan(f, s);
    }

    bool setData(const QModelIndex& index, const QVariant& data, qint32 role) override
    {
        if(index.column() == 0 && role == Qt::CheckStateRole) {
            Selection component = m_extractor(mapToSource(index));
            if(data.value<Qt::CheckState>() == Qt::Checked) {
                Selected.Insert(component);
            } else {
                Selected.Remove(component);
            }
            return true;
        }
        return Super::setData(index, data, role);
    };

    QVariant data(const QModelIndex& index, qint32 role) const override {
        if(index.column() == 0 && role == Qt::CheckStateRole) {
            auto selection = m_extractor(mapToSource(index));
            if(selection != 0) {
                return Selected.IsContains(selection) ? Qt::Checked : Qt::Unchecked;
            }
            return QVariant();
        }
        return Super::data(index, role);
    };

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        auto defaultFlags = Super::flags(index);
        if(index.column() == 0) {
            return defaultFlags |= Qt::ItemIsUserCheckable;
        }
        return defaultFlags;
    };

    void CreateDefaultActions(class QTableView* table)
    {
        MenuWrapper MenuWrapper(table);
        MenuWrapper.AddAction(tr("Check Selected"), [this, table](QAction*){
            emit layoutAboutToBeChanged();
            Select(table->selectionModel()->selectedIndexes());
            emit layoutChanged();
        })->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Insert));
        MenuWrapper.AddAction(tr("Uncheck selected"), [this, table](QAction*){
            emit layoutAboutToBeChanged();
            Deselect(table->selectionModel()->selectedIndexes());
            emit layoutChanged();
        })->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    }

private:
    FExtractor m_extractor;
};

#endif // MODELSFILTERMODELBASE_H
