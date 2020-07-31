#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <SharedModule/internal.hpp>

class AbstractTableModel : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    using Super::Super;

    static AbstractTableModel* Wrap(QAbstractTableModel* model) { return reinterpret_cast<AbstractTableModel*>(model); }

private:
    friend class ModelsTableWrapper;
};

class ModelsTableWrapper
{
public:
    virtual ~ModelsTableWrapper()
    {
        OnDestroyed();
    }
    void ConnectModel(AbstractTableModel* model);
    void DisconnectModel(AbstractTableModel* model);

    CommonDispatcher<qint32,qint32> OnValueChanged;
    Dispatcher OnAboutToBeReseted;
    Dispatcher OnReseted;
    Dispatcher OnAboutToBeUpdated;
    Dispatcher OnUpdated;
    CommonDispatcher<qint32,qint32> OnAboutToRemoveRows;
    Dispatcher OnRowsRemoved;
    CommonDispatcher<qint32,qint32> OnAboutToInsertRows;
    Dispatcher OnRowsInserted;
    Dispatcher OnDestroyed;
};

inline void ModelsTableWrapper::ConnectModel(AbstractTableModel* model)
{
    OnValueChanged += { model, [model](qint32 row, qint32 column) {
        auto modelIndex = model->index(row, column);
        emit model->dataChanged(modelIndex, modelIndex); }
    };
    OnAboutToBeReseted += { model, [model]{ model->beginResetModel(); }};
    OnReseted += { model, [model]{ model->endResetModel(); } };
    OnAboutToBeUpdated += { model, [model]{ emit model->layoutAboutToBeChanged(); }};
    OnUpdated += { model, [model]{ emit model->layoutAboutToBeChanged(); }};
    OnAboutToRemoveRows += { model, [model](qint32 start,qint32 end){ model->beginRemoveRows(QModelIndex(), start, end); } };
    OnRowsRemoved += { model, [model]{ model->endRemoveRows(); } };
    OnAboutToInsertRows += { model, [model](qint32 start,qint32 end){ model->beginInsertRows(QModelIndex(), start, end); } };
    OnRowsInserted += { model, [model]{ model->endInsertRows(); }};
}

inline void ModelsTableWrapper::DisconnectModel(AbstractTableModel* model)
{
    OnValueChanged -= model;
    OnAboutToBeReseted -= model;
    OnReseted -= model;
    OnAboutToBeUpdated -= model;
    OnUpdated -= model;
    OnAboutToRemoveRows -= model;
    OnRowsRemoved -= model;
    OnAboutToInsertRows -= model;
    OnRowsInserted -= model;
    OnDestroyed -= model;
}

#endif // WRAPPERS_H
