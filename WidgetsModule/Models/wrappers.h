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
};

inline void ModelsTableWrapper::ConnectModel(QAbstractTableModel* qmodel)
{
    auto* model = ModelsAbstractTableModel::Wrap(qmodel);
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
}

#endif // WRAPPERS_H
