#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <SharedModule/internal.hpp>

class ModelsAbstractTableModel : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
    using Super::Super;

    friend class ModelsTableWrapper;
    static ModelsAbstractTableModel* Wrap(QAbstractTableModel* model) { return reinterpret_cast<ModelsAbstractTableModel*>(model); }
};

class ModelsTableWrapper
{
public:
    virtual ~ModelsTableWrapper()
    {
        OnAboutToBeDestroyed();
    }
    void ConnectModel(QAbstractTableModel* model);
    void DisconnectModel(QAbstractTableModel* model);

    CommonDispatcher<qint32,qint32> OnValueChanged;
    Dispatcher OnAboutToBeReseted;
    Dispatcher OnReseted;
    Dispatcher OnAboutToBeUpdated;
    Dispatcher OnUpdated;
    CommonDispatcher<qint32,qint32> OnAboutToRemoveRows;
    Dispatcher OnRowsRemoved;
    CommonDispatcher<qint32,qint32> OnAboutToInsertRows;
    Dispatcher OnRowsInserted;
    Dispatcher OnAboutToBeDestroyed;
    Dispatcher OnChanged;
    CommonDispatcher<qint32> OnAboutToChangeRow;
    CommonDispatcher<qint32> OnRowChanged;
};

using ModelsTableWrapperPtr = SharedPointer<ModelsTableWrapper>;

inline void ModelsTableWrapper::ConnectModel(QAbstractTableModel* qmodel)
{
    auto* model = ModelsAbstractTableModel::Wrap(qmodel);
    OnValueChanged += { model, [model](qint32 row, qint32 column) {
        auto modelIndex = model->index(row, column);
        emit model->dataChanged(modelIndex, modelIndex);
    }};
    OnAboutToBeReseted += { model, [model]{ model->beginResetModel(); }};
    OnReseted += { model, [model]{ model->endResetModel(); } };
    OnAboutToBeUpdated += { model, [model]{ emit model->layoutAboutToBeChanged(); }};
    OnUpdated += { model, [model]{ emit model->layoutChanged(); }};
    OnAboutToRemoveRows += { model, [model](qint32 start,qint32 end){ model->beginRemoveRows(QModelIndex(), start, end); } };
    OnRowsRemoved += { model, [model]{ model->endRemoveRows(); } };
    OnAboutToInsertRows += { model, [model](qint32 start,qint32 end){ model->beginInsertRows(QModelIndex(), start, end); } };
    OnRowsInserted += { model, [model]{ model->endInsertRows(); }};
    OnRowChanged += { model, [model] (qint32 row){
        auto startmi = model->index(row, 0);
        auto endmi = model->index(row, model->columnCount());
        model->dataChanged(startmi, endmi);
    }};
}

inline void ModelsTableWrapper::DisconnectModel(QAbstractTableModel* qmodel)
{
    auto* model = ModelsAbstractTableModel::Wrap(qmodel);
    OnValueChanged -= model;
    OnAboutToBeReseted -= model;
    OnReseted -= model;
    OnAboutToBeUpdated -= model;
    OnUpdated -= model;
    OnAboutToRemoveRows -= model;
    OnRowsRemoved -= model;
    OnAboutToInsertRows -= model;
    OnRowsInserted -= model;
    OnAboutToBeDestroyed -= model;
    OnRowChanged -= model;
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

#endif // WRAPPERS_H
