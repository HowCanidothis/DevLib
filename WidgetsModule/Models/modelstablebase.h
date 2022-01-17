#ifndef MODELSTABLEBASE_H
#define MODELSTABLEBASE_H

#include <QAbstractTableModel>

#include "wrappers.h"
#include "WidgetsModule/Utils/iconsmanager.h"

struct ModelsIconsContext
{
    IconsSvgIcon ErrorIcon;
    IconsSvgIcon WarningIcon;
    IconsSvgIcon InfoIcon;

private:
    friend class ViewModelsTableBase;
    ModelsIconsContext();
};

class ViewModelsTableColumnComponents
{
public:
    struct ColumnComponentData
    {
        // Note. By Default handlers does not propagate, set ignore to true to force propagation if needed
        using FSetterHandler = std::function<bool (const QModelIndex& index, const QVariant& data, bool& )>;
        using FGetterHandler = std::function<QVariant (const QModelIndex& index, bool& accept)>;
        using FGetHeaderHandler = std::function<QVariant (bool& accept)>;
        FSetterHandler SetterHandler;
        FGetterHandler GetterHandler;
        FGetHeaderHandler GetHeaderHandler;

        ColumnComponentData()
            : SetterHandler([](const QModelIndex&, const QVariant&, bool&) -> bool { return false; })
            , GetterHandler([](const QModelIndex&, bool& ) -> QVariant { return QVariant(); })
            , GetHeaderHandler([](bool& ) -> QVariant { return QVariant(); })
        {}

        ColumnComponentData(bool propagate)
            : SetterHandler([](const QModelIndex&, const QVariant&, bool& accepted) -> bool { accepted = false; return false; })
            , GetterHandler([](const QModelIndex&, bool& accepted) -> QVariant { accepted = false; return QVariant(); })
            , GetHeaderHandler([](bool& accepted) -> QVariant { accepted = false; return QVariant(); })
        {}

        ColumnComponentData& SetSetter(const FSetterHandler& setter)
        {
            SetterHandler = setter;
            return *this;
        }
        ColumnComponentData& SetGetter(const FGetterHandler& setter)
        {
            GetterHandler = setter;
            return *this;
        }
        ColumnComponentData& SetHeader(const FGetHeaderHandler& setter)
        {
            GetHeaderHandler = setter;
            return *this;
        }
    };

    struct ColumnFlagsComponentData
    {
        std::function<Qt::ItemFlags (qint32 row, bool& ignore)> GetFlagsHandler = [](qint32, bool&) { return Qt::NoItemFlags; };
    };

    ViewModelsTableColumnComponents();

    template<class T> using FModelGetter = std::function<QVariant (const typename T::value_type&)>;
    template<class T> using FModelSetter = std::function<FAction (const QVariant& data, typename T::value_type&)>;

    template<class Wrapper>
    struct DescAddDefaultModelSourceComponentParams
    {
        FModelGetter<Wrapper> GetterUi;
        FModelGetter<Wrapper> Getter;
        FModelSetter<Wrapper> Setter;

        DescAddDefaultModelSourceComponentParams(const FModelGetter<Wrapper>& getterUi)
            : GetterUi(getterUi)
        {

        }

        DescAddDefaultModelSourceComponentParams& SetGetter(const FModelGetter<Wrapper>& getter)
        {
            Getter = getter;
            return *this;
        }
        DescAddDefaultModelSourceComponentParams& SetSetter(const std::function<void(typename Wrapper::value_type&, const QVariant&)>& handler){
            Setter = [handler](const QVariant& value, typename Wrapper::value_type& data){ return [&]{ handler(data, value);}; };
            return *this;
        }
        DescAddDefaultModelSourceComponentParams& SetSetter(const FModelSetter<Wrapper>& setter)
        {
            Setter = setter;
            return *this;
        }
    };

    template<class Wrapper>
    void AddDefaultModelSourceComponent(qint32 column, const FTranslationHandler& header, const std::function<Wrapper* ()>& getterModel, const DescAddDefaultModelSourceComponentParams<Wrapper>& inParams)
    {
        ColumnComponentData displayRoleComponent;
        auto params = inParams;
        if(params.Getter == nullptr) {
            params.Getter = params.GetterUi;
        }

        displayRoleComponent.GetterHandler = [params, getterModel](const QModelIndex& index, bool&) -> QVariant {
            if(index.row() >= getterModel()->GetSize()) {
                return "-";
            }
            return params.GetterUi(getterModel()->At(index.row()));
        };
        displayRoleComponent.GetHeaderHandler = [header](bool&){ return header(); };

        AddComponent(Qt::DisplayRole, column, displayRoleComponent);
        if(params.Setter != nullptr) {
            ColumnComponentData editRoleComponent;

            editRoleComponent.GetterHandler = [params, getterModel](const QModelIndex& index, bool&) -> QVariant {
                if(index.row() >= getterModel()->GetSize()) {
                    return QVariant();
                }
                return params.Getter(getterModel()->At(index.row()));
            };
            editRoleComponent.SetterHandler = [params, getterModel](const QModelIndex& index, const QVariant& data, bool&) -> bool {
                if(index.row() >= getterModel()->GetSize()) {
                    return false;
                }
                return getterModel()->EditWithCheck(index.row(), [&](auto& value){ return params.Setter(data, value); });
            };

            AddComponent(Qt::EditRole, column, editRoleComponent);
            AddFlagsComponent(column, { [](qint32, bool&) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable; } });
        } else {
            AddFlagsComponent(column, { [](qint32, bool&) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable; } });
        }
    }

    void AddComponent(Qt::ItemDataRole role, qint32 column, const ColumnComponentData& columnData);
    void AddFlagsComponent(qint32 column, const ColumnFlagsComponentData& flagsColumnData);

    bool SetData(const QModelIndex& index, const QVariant& data, qint32 role, bool& result);
    bool GetData(const QModelIndex& index, qint32 role, QVariant& data) const;
    bool GetHeaderData(qint32 section, qint32 role, QVariant& header) const;
    bool GetFlags(const QModelIndex& index, Qt::ItemFlags& flags) const;

    qint32 GetColumnCount() const;

private:
    bool callHandler(qint32 column, Qt::ItemDataRole role, const std::function<void (const QVector<ColumnComponentData>& )>& onFound) const;
    bool callFlagsHandler(qint32 column, const std::function<void (const QVector<ColumnFlagsComponentData>& )>& onFound) const;

private:
    QMap<qint32, QHash<Qt::ItemDataRole, QVector<ColumnComponentData>>> m_columnComponents;
    QMap<qint32, QVector<ColumnFlagsComponentData>> m_columnFlagsComponents;
};

class ViewModelsTableBase : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    ViewModelsTableBase(QObject* parent = nullptr);
    ~ViewModelsTableBase();

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, qint32 role) const override;
    bool setData(const QModelIndex& index, const QVariant& data, qint32 role) override;
    QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role) const override;
    qint32 columnCount(const QModelIndex&) const override
    {
        return ColumnComponents.GetColumnCount();
    }

    void AttachDependence(Dispatcher* dispatcher, int first, int last)
    {
        dispatcher->Connect(this, [first, last, this]{
            emit dataChanged(createIndex(0, first), createIndex(rowCount()-1, last));
            emit headerDataChanged(Qt::Horizontal, first, last);
        }).MakeSafe(m_connections);
    }

    void SetData(const ModelsTableWrapperPtr& data);
    const ModelsTableWrapperPtr& GetData() const { return m_data; }
    const ModelsIconsContext& GetIconsContext() const { return m_iconsContext; }

    Dispatcher OnModelChanged;

    ViewModelsTableColumnComponents ColumnComponents;

protected:
    ModelsTableWrapperPtr m_data;
    ModelsIconsContext m_iconsContext;
    QHash<qint32, std::function<QVariant (qint32 row, qint32 column)>> m_roleDataHandlers;
    QHash<qint32, std::function<QVariant (qint32 column)>> m_roleHorizontalHeaderDataHandlers;
    QHash<qint32, std::function<QVariant (qint32 column)>> m_roleVerticalHeaderDataHandlers;
    QHash<qint32, std::function<bool (qint32 row, qint32 column, const QVariant&)>> m_roleSetDataHandlers;
    DispatcherConnectionsSafe m_connections;
};

template<class T>
class TViewModelsTableBase : public ViewModelsTableBase
{
    using Super = ViewModelsTableBase;
public:
    using Super::Super;

    qint32 rowCount(const QModelIndex&  = QModelIndex()) const override
    {
        return GetData() != nullptr ? GetData()->GetSize() : 0;
    }
    bool insertRows(int row, int count, const QModelIndex& = QModelIndex()) override
	{
        Q_ASSERT(GetData() != nullptr);
        GetData()->Insert(row > GetData()->GetSize() ? GetData()->GetSize() : row, count);
		return true;
	}
    bool removeRows(int row, int count, const QModelIndex& = QModelIndex()) override
	{
        Q_ASSERT(GetData() != nullptr);
		QSet<qint32> indexs;
		while(count){
            indexs.insert(row + --count);
		}
		GetData()->Remove(indexs);
		return true;
	}
	
    void SetData(const SharedPointer<T>& data) { Super::SetData(data); }
    const SharedPointer<T>& GetData() const { return Super::GetData().template Cast<T>(); }

protected:
    bool isLastEditRow(const QModelIndex& index) const
    {
        Q_ASSERT(GetData() != nullptr);
        return GetData()->GetSize() == index.row();
    }
};

template<class Wrapper>
struct ModelsTableBaseDecorator
{
    /// deprecated
    static QVariant GetModelData(const typename Wrapper::value_type& data, qint32 column, qint32 role = Qt::DisplayRole);
    static void Sort(const typename Wrapper::container_type& rows, qint32 column, Array<qint32>& indices);
    /// deprecated
    static bool SetModelData(const QVariant& value, typename Wrapper::value_type& data, qint32 column, qint32 role = Qt::DisplayRole);
    /// deprecated
    static Qt::ItemFlags GetFlags(const typename Wrapper::value_type&, qint32);
    /// deprecated
    static QVariant GetHeaderData(int section, Qt::Orientation orientation, qint32 role = Qt::DisplayRole);
};

template<class Wrapper>
struct ModelsTableDecoratorHelpers
{
    template<qint32 Column>
    static void Sort(const Wrapper& rows, Array<qint32>& indices)
    {
        std::sort(indices.begin(), indices.end(), [&](qint32 f, qint32 s){
            return rows.At(f).template Get<Column>() < rows.At(s).template Get<Column>();
        });
    }
};

template<class Wrapper>
class TViewModelsDecoratedTable : public TViewModelsTableBase<Wrapper>
{
    using Super = TViewModelsTableBase<Wrapper>;
public:
    using Super::Super;

    qint32 columnCount(const QModelIndex&  = QModelIndex()) const override
    {
        return std::max(Super::ColumnComponents.GetColumnCount(), (qint32)Wrapper::value_type::count);
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if(!index.isValid()) {
            return QVariant();
        }
        Q_ASSERT(Super::GetData() != nullptr);

        QVariant result;
        if(Super::ColumnComponents.GetData(index, role, result)) {
            return result;
        }

        return ModelsTableBaseDecorator<Wrapper>::GetModelData(Super::GetData()->At(index.row()), index.column(), role);
    }
    bool setData(const QModelIndex& index, const QVariant& data, qint32 role = Qt::DisplayRole) override
    {
        bool result = false;
        if(!index.isValid()) {
            return result;
        }

        Q_ASSERT(Super::GetData() != nullptr);

        if(Super::ColumnComponents.SetData(index, data, role, result)) {
            return result;
        }

        Super::GetData()->Edit(index.row(), [&](typename Wrapper::value_type& value){
            result = ModelsTableBaseDecorator<Wrapper>::SetModelData(data, value, index.column(), role);
        }, {index.column()});
        return result;
    }

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override 
	{
        QVariant result;
        if(Super::ColumnComponents.GetHeaderData(section, role, result)) {
            return result;
        }

		return ModelsTableBaseDecorator<Wrapper>::GetHeaderData(section, orientation, role);
	}
	
    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if(!index.isValid()) {
            return Qt::NoItemFlags;
        }
        Qt::ItemFlags result;
        if(Super::ColumnComponents.GetFlags(index, result)) {
            return result;
        }
        return ModelsTableBaseDecorator<Wrapper>::GetFlags(Super::GetData()->At(index.row()), index.column());
    }
};

template<class Wrapper>
class ModelsTableSearchComponent
{
    struct Cache
    {
        Array<qint32> SortedData;
        bool IsValid = false;
        Dispatcher ColumnListener;
    };

public:
    template<class T, qint32 Column>
    class iterator {
        qint32 m_row;
        ModelsTableSearchComponent* m_searchComponent;
        ModelsTableSearchComponent::Cache* m_cache;

        friend class ModelsTableSearchComponent;

        iterator(qint32 row, ModelsTableSearchComponent* searchComponent)
            : m_row(row)
            , m_searchComponent(searchComponent)
            , m_cache(&searchComponent->m_sortedColumns[Column])
        {

        }
    public:
        typedef iterator It;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;
        typedef T value_type;
        typedef const T* pointer;
        typedef const T& reference;

        bool operator!=(const It& other) const{ return this->m_row != other.m_row; }
        It& operator++()
        {
            ++m_row;
            return *this;
        }
        It operator++(qint32)
        {
            ++m_row;
            return *this;
        }
        It& operator--()
        {
            --m_row;
            return *this;
        }
        It operator+(qint32 index)
        {
            It n(*this);
            n.m_row += index;
            return n;
        }
        It& operator+=(qint32 index)
        {
            m_row += index;
            return *this;
        }
        friend size_t operator-(const It& first, const It& another)
        {
            return first.m_row - another.m_row;
        }
        reference operator*() const { Q_ASSERT(IsValid()); return m_searchComponent->m_wrapper->At(m_cache->SortedData[m_row]).template Get<Column>(); }
        pointer operator->() const { Q_ASSERT(IsValid()); return &m_searchComponent->m_wrapper->At(m_cache->SortedData[m_row]).template Get<Column>(); }
        reference value() const { return operator*(); }
        bool operator==(const It& other) const { return !(operator !=(other)); }
        bool IsValid() const { return m_row >= 0 && m_row < m_cache->SortedData.Size(); }
        qint32 GetRowIndex() const { return m_row; }
        const typename Wrapper::value_type& GetRow() const { Q_ASSERT(IsValid()); return m_searchComponent->m_wrapper->At(m_cache->SortedData[m_row]); }
        qint32 GetSourceRowIndex() const { Q_ASSERT(IsValid()); return m_cache->SortedData[m_row]; }
    };

    using value_type = typename Wrapper::value_type;
    using const_iterator = typename Wrapper::container_type::const_iterator;
    ModelsTableSearchComponent(const SharedPointer<Wrapper>& wrapper)
        : m_wrapper(wrapper)
    {
        auto invalidate = [this](const QSet<qint32>& columns){
            if(columns.isEmpty()) {
                for(qint32 i(0); i < value_type::count; i++) {
                    auto& cache = m_sortedColumns[i];
                    cache.IsValid = false;
                    cache.ColumnListener();
                }
            } else {
                for(qint32 i : columns) {
                    if(i<Wrapper::value_type::count){
                        auto& cache = m_sortedColumns[i];
                        cache.IsValid = false;
                        cache.ColumnListener();
                    }
                }
            }
        };

        m_wrapper->OnColumnsChanged.Connect(const_cast<ModelsTableSearchComponent*>(this), invalidate).MakeSafe(m_connections);
    }

    QList<const value_type&> Select(const std::function<bool(const value_type&)>& where)
    {
        QList<const value_type&> result;
        for(const auto& row : *m_wrapper){
            if(where(row)){
                result.append(row);
            }
        }
        return result;
    }
    
    template<class T, qint32 Column>
    iterator<T, Column> FindEqualOrGreater(const T& value) const
    {
        return std::lower_bound(begin<T,Column>(), end<T,Column>(), value, [](const T& f, const T&s){
            return f < s;
        });
    }

    template<class T, qint32 Column>
    iterator<T, Column> FindEqualOrLower(const T& value) const
    {
        auto foundIt = FindEqualOrGreater<T,Column>(value);
        if(!foundIt.IsValid() || foundIt.value() > value) {
            --foundIt;
        }
        return foundIt;
    }

    void Sort(qint32 column) const
    {
        Q_ASSERT(column >= 0 && column < value_type::count);
        auto& cache = m_sortedColumns[column];
        if(!cache.IsValid) {
            ModelsTableBaseDecorator<Wrapper>::Sort(*m_wrapper, column, cache.SortedData);
            cache.IsValid = true;
        }
    }

    template<qint32 Column>
    DispatcherConnection Bind(const FAction& action) const
    {
        return m_sortedColumns[Column].ColumnListener.Connect(const_cast<ModelsTableSearchComponent*>(this), action);
    }

    template<class T, qint32 Column>
    iterator<T, Column> begin() const
    {
        Sort(Column);
        return iterator<T, Column>(0, const_cast<ModelsTableSearchComponent*>(this));
    }

    template<class T, qint32 Column>
    iterator<T, Column> end() const
    {
        Sort(Column);
        return iterator<T, Column>(m_sortedColumns[Column].SortedData.Size(), const_cast<ModelsTableSearchComponent*>(this));
    }

    template<class T, qint32 Column>
    adapters::Range<iterator<T, Column>> range() const
    {
        return adapters::range(begin<T,Column>(), end<T,Column>());
    }

    const SharedPointer<Wrapper>& GetSource() const { return m_wrapper; }

private:
    template<class T, qint32> friend class iterator;
    SharedPointer<Wrapper> m_wrapper;
    mutable Cache m_sortedColumns[Wrapper::value_type::count];
    DispatcherConnectionsSafe m_connections;
};

namespace widget_models
{

template<qint32 Column>
struct VisitorHelper
{
    template<class Wrapper>
    static void Sort(const Wrapper& rows, Array<qint32>& indices)
    {
        std::sort(indices.begin(), indices.end(), [&](qint32 f, qint32 s){
            return rows.At(f).template Get<Column>() < rows.At(s).template Get<Column>();
        });
    }
};

template <std::size_t L, std::size_t U>
struct Visitor
{
    template <class Wrapper>
    static void Sort(const Wrapper& rows, Array<qint32>& indices, std::size_t idx)
    {
        static constexpr std::size_t MEDIAN = (U - L) / 2 + L;
        if (idx > MEDIAN)
            Visitor<MEDIAN, U>::Sort(rows, indices, idx);
        else if (idx < MEDIAN)
            Visitor<L, MEDIAN>::Sort(rows, indices, idx);
        else
            VisitorHelper<MEDIAN>::Sort(rows, indices);
    }
};

template <class Wrapper>
void sort(const Wrapper& rows, Array<qint32>& indices, std::size_t idx)
{
    assert(idx <= Wrapper::value_type::count);
    Visitor<0, Wrapper::value_type::count>::Sort(rows, indices, idx);
}

}

#endif // MODELSTABLEBASE_H
