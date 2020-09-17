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
        OnRowsInserted.Connect(this, [this] { testOutScope(); });
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
    Dispatcher OnRowsInserted;
    Dispatcher OnAboutToBeDestroyed;
    Dispatcher OnChanged;
    CommonDispatcher<qint32> OnAboutToChangeRow;
    CommonDispatcher<qint32> OnRowChanged;

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
    OnRowsInserted += { model, [model]{ model->endInsertRows(); }};
    OnRowChanged += { model, [model] (qint32 row){
        auto startmi = model->index(row, 0);
        auto endmi = model->index(row, model->columnCount());
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
    OnRowChanged -= model;
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

    CommonDispatcher<ModelsTreeItemBase*,QVector<int>> OnTreeValueChanged;

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
    OnTreeValueChanged += {model, [model](ModelsTreeItemBase* item, QVector<int> roles){
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
class TModelsTableWrapper : private Container, public ModelsTableWrapper
{
    using Super = Container;
public:
    using value_type = typename Super::value_type;
    using Super::Super;

    void Swap(Super& another)
    {
        OnAboutToBeReseted();
        Super::swap(another);
        OnReseted();
        OnChanged();
    }

    void Append(const value_type& part)
    {
        auto size = GetSize();
        OnAboutToInsertRows(size, size);
        Super::append(part);
        OnRowsInserted();
        OnChanged();
    }

    void Remove(const QSet<qint32>& indexes)
    {
        OnAboutToBeReseted();
        qint32 currentIndex = 0;
        auto endIt = std::remove_if(Super::begin(), Super::end(), [&currentIndex, &indexes](const value_type& ){
            return indexes.contains(currentIndex++);
        });
        Super::resize(std::distance(Super::begin(), endIt));
        OnReseted();
        OnChanged();
    }

    void Edit(qint32 index, const std::function<void (value_type& value)>& handler)
    {
        OnAboutToChangeRow(index);
        handler(Super::operator[](index));
        OnRowChanged(index);
        OnChanged();
    }

    qint32 IndexOf(const value_type& value) const
    {
        return Super::indexOf(value);
    }

    Super& EditSilent() { return *this; }

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
