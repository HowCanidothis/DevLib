#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <QAbstractItemModel>

#include <SharedModule/internal.hpp>

#include "modelstreeitembase.h"

class ModelsAbstractItemModel : public QAbstractItemModel
{
    using Super = QAbstractItemModel;
    using Super::Super;

    friend class ModelsWrapperBase;
    friend class ModelsTableWrapper;
    friend class ModelsTreeWrapper;
    static ModelsAbstractItemModel* Wrap(QAbstractItemModel* model) { return reinterpret_cast<ModelsAbstractItemModel*>(model); }
};

class ModelsWrapperBase
{
public:
    ModelsWrapperBase()
        : m_inScope(false)
    {
        OnAboutToBeReseted.Connect(this, [this] { testInScope(); });
        OnAboutToBeUpdated.Connect(this, [this] { testInScope(); });
        OnReseted.Connect(this, [this] { testOutScope(); });
        OnUpdated.Connect(this, [this] { testOutScope(); });
        OnRowsRemoved.Connect(this, [this] { testOutScope(); });
        OnRowsInserted.Connect(this, [this] (qint32, qint32) { testOutScope(); });
    }

    virtual ~ModelsWrapperBase()
    {
        OnAboutToBeDestroyed();
    }
    virtual void ConnectModel(QAbstractItemModel* model);
    virtual void DisconnectModel(QAbstractItemModel* model);

    bool IsInScope() const { return m_inScope; }

    Dispatcher OnAboutToBeReseted;
    Dispatcher OnReseted;
    Dispatcher OnAboutToBeUpdated;
    Dispatcher OnUpdated;
    Dispatcher OnRowsRemoved;
    CommonDispatcher<qint32, qint32> OnRowsInserted;
    Dispatcher OnAboutToBeDestroyed;
    Dispatcher OnChanged;
    CommonDispatcher<QSet<qint32>> OnColumnsChanged;
    CommonDispatcher<qint32, qint32> OnAboutToChangeRows;
    CommonDispatcher<qint32, qint32, const QSet<qint32>&> OnRowsChanged;

protected:
    void testInScope()
    {
        Q_ASSERT(!m_inScope);
        m_inScope = true;
    }

    void testOutScope()
    {
        Q_ASSERT(m_inScope);
        m_inScope = false;
    }

private:
    bool m_inScope;
};

inline void ModelsWrapperBase::ConnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);

    OnAboutToBeReseted += { model, [model]{
        model->beginResetModel();
    }};
    OnReseted += { model, [model]{
        model->endResetModel();
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

inline void ModelsWrapperBase::DisconnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);

    OnAboutToBeReseted -= model;
    OnReseted -= model;
    OnAboutToBeUpdated -= model;
    OnUpdated -= model;
    OnRowsRemoved -= model;
    OnRowsInserted -= model;
    OnAboutToBeDestroyed -= model;
    OnRowsChanged -= model;
}

class ModelsTreeWrapper : public ModelsWrapperBase
{
    using Super = ModelsWrapperBase;
public:
    ModelsTreeWrapper()
    {
        OnAboutToInsertRows.Connect(this, [this](qint32,qint32, ModelsTreeItemBase*){ testInScope(); });
        OnAboutToRemoveRows.Connect(this, [this](qint32,qint32, ModelsTreeItemBase*){ testInScope(); });
    }
    void ConnectModel(QAbstractItemModel* model) override;
    void DisconnectModel(QAbstractItemModel* model) override;

    CommonDispatcher<qint32,qint32,ModelsTreeItemBase*> OnAboutToRemoveRows;
    CommonDispatcher<qint32,qint32,ModelsTreeItemBase*> OnAboutToInsertRows;

    CommonDispatcher<size_t, ModelsTreeItemBase*,QVector<int>> OnTreeValueChanged;

    virtual ModelsTreeItemBase* GetRoot() = 0;
};

inline void ModelsTreeWrapper::ConnectModel(QAbstractItemModel* qmodel)
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

inline void ModelsTreeWrapper::DisconnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);
    Super::DisconnectModel(qmodel);
    OnAboutToRemoveRows -= model;
    OnAboutToInsertRows -= model;
    OnTreeValueChanged -= model;
}

class ModelsTableWrapper : public ModelsWrapperBase
{
    using Super = ModelsWrapperBase;
public:
    ModelsTableWrapper()
    {
        OnAboutToInsertRows.Connect(this, [this](qint32,qint32){ testInScope(); });
        OnAboutToRemoveRows.Connect(this, [this](qint32,qint32){ testInScope(); });
    }
    void ConnectModel(QAbstractItemModel* model) override;
    void DisconnectModel(QAbstractItemModel* model) override;

    CommonDispatcher<qint32,qint32> OnAboutToRemoveRows;
    CommonDispatcher<qint32,qint32> OnAboutToInsertRows;
    CommonDispatcher<qint32,qint32> OnValueChanged;
};

inline void ModelsTableWrapper::ConnectModel(QAbstractItemModel* qmodel)
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

inline void ModelsTableWrapper::DisconnectModel(QAbstractItemModel* qmodel)
{
    auto* model = ModelsAbstractItemModel::Wrap(qmodel);
    Super::DisconnectModel(qmodel);
    OnAboutToRemoveRows -= model;
    OnAboutToInsertRows -= model;
    OnValueChanged -= model;
}

template<class Container>
class TModelsTableWrapper : protected Container, public ModelsTableWrapper
{
    using Super = Container;
public:
    using container_type = Container;
    using value_type = typename Super::value_type;
    using Super::Super;

    TModelsTableWrapper<Container>& BaseWrapper() { return *this; }

    void Swap(Super& another)
    {
        OnAboutToBeReseted();
        Super::swap(another);
        OnReseted();
        OnChanged();
        OnColumnsChanged({});
    }

    void Clear() {
        OnAboutToBeReseted();
        Super::clear();
        OnReseted();
        OnChanged();
        OnColumnsChanged({});
    }

    bool IsValidRow(qint32 row) const
    {
        return row >= 0 && row < GetSize();
    }

    qint32 Find(const value_type& value) const
    {
        return Find([&value](const value_type& current){ return value == current; });
    }

    qint32 Find(const std::function<bool (const value_type&)>& searchPredicate) const
    {
        auto it = Super::begin();
        while(it != Super::end()) {
            if(searchPredicate(*it)) {
                return std::distance(Super::begin(), it);
            }
            it++;
        }
        return -1;
    }

    virtual void Prepend(const value_type& part)
    {
        OnAboutToInsertRows(0, 0);
        Super::prepend(part);
        OnRowsInserted(0, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    virtual void Append(const value_type& part)
    {
        auto size = GetSize();
        OnAboutToInsertRows(size, size);
        Super::append(part);
        OnRowsInserted(size, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    void InsertSorted(const value_type& value, const std::function<bool (const value_type& f, const value_type& s )>& lessThan = [](const value_type& f, const value_type& s ){ return f < s; })
    {
        auto insertTo = std::lower_bound(begin(), end(), value, lessThan);
        auto index = std::distance(begin(), insertTo);
        OnAboutToInsertRows(index, index);
        Super::insert(index, value);
        OnRowsInserted(index, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    template<class T2, typename F = bool (*)(const value_type&, const T2&)>
    typename Super::const_iterator FindSorted(const T2& value, const F& lessThan) const
    {
        return std::lower_bound(begin(), end(), value, lessThan);
    }

	void Insert(int index, int count, const value_type& part = value_type())
	{
		OnAboutToInsertRows(index, index + count - 1);
        Super::insert(index, count, part);
        OnRowsInserted(index, count);
        OnChanged();
        OnColumnsChanged({});
	}

    void Insert(int index, qint32 count, const std::function<void (qint32 index, value_type& data)>& dataInitializer)
    {
        OnAboutToInsertRows(index, index + count - 1);
        Super::insert(index, count, value_type());
        qint32 i = index;
        for(auto& value : adapters::range(Super::begin() + index, Super::begin() + index + count)) {
            dataInitializer(i, value);
        }
        OnRowsInserted(index, count);
        OnChanged();
        OnColumnsChanged({});
    }
	
    void Insert(int index, const value_type& part)
    {
        OnAboutToInsertRows(index, index);
        Super::insert(index, part);
        OnRowsInserted(index, 1);
        OnChanged();
        OnColumnsChanged({});
    }

    void UpdateUi(const FAction& handler)
    {
        OnAboutToBeUpdated();
        handler();
        OnUpdated();
    }

    void Update(const FAction& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeUpdated();
        handler();
        OnUpdated();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Change(const std::function<void (Container&)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeReseted();
        handler(EditSilent());
        OnReseted();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Change(const FAction& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToBeReseted();
        handler();
        OnReseted();
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }


    void Remove(const QSet<qint32>& indexes)
    {
        OnAboutToBeReseted();
        qint32 currentIndex = 0;
        QVector<value_type> toRemove;
        auto endIt = std::remove_if(Super::begin(), Super::end(), [&currentIndex, &indexes, &toRemove](const value_type& a){
            if(indexes.contains(currentIndex++)) {
                toRemove.append(a);
                return true;
            }
            return false;
        });
        Super::resize(std::distance(Super::begin(), endIt));
        OnReseted();
        OnChanged();
        OnColumnsChanged({});
    }

    void Edit(qint32 index, qint32 count, const std::function<void (int, value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToChangeRows(index, count);
        for(int i=0; i<count; ++i){
            handler(i, Super::operator[](index + i));
        }
        OnRowsChanged(index, count, affectedColumns);
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    void Edit(qint32 index, const std::function<void (value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        OnAboutToChangeRows(index, 1);
        handler(Super::operator[](index));
        OnRowsChanged(index, 1, affectedColumns);
        OnChanged();
        OnColumnsChanged(affectedColumns);
    }

    bool EditWithCheck(qint32 index, const std::function<FAction (value_type& value)>& handler, const QSet<qint32>& affectedColumns = QSet<qint32>())
    {
        auto updateFunction = handler(Super::operator[](index));
        if(updateFunction != nullptr) {
            OnAboutToChangeRows(index, 1);
            updateFunction();
            OnRowsChanged(index, 1, affectedColumns);
            OnChanged();
            OnColumnsChanged(affectedColumns);
            return true;
        }
        return false;
    }

    qint32 IndexOf(const value_type& value) const
    {
        return Super::indexOf(value);
    }

    Super& EditSilent() { return *this; }
    const Super& Native() const { return *this; }

    const value_type& First() const { return Super::first(); }
    const value_type& Last() const { return Super::last(); }
    bool IsEmpty() const { return Super::isEmpty(); }
    qint32 GetSize() const { return Super::size(); }

    const value_type& At(qint32 index) const { return Super::at(index); }
    const value_type& operator[](qint32 index) const { return Super::operator[](index); }

    typename Super::const_iterator begin() const { return Super::begin(); }
    typename Super::const_iterator end() const { return Super::end(); }
};

using ModelsTableWrapperPtr = SharedPointer<ModelsTableWrapper>;
using ModelsTreeWrapperPtr = SharedPointer<ModelsTreeWrapper>;

#endif // WRAPPERS_H
