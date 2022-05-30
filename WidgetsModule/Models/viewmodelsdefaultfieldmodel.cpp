#include "ViewModelsDefaultFieldModel.h"

ViewModelsDefaultFieldModel::ViewModelsDefaultFieldModel(QObject* parent)
    : Super(parent)
    , m_sourceModel(nullptr)
{}

void ViewModelsDefaultFieldModel::SetDefaultFieldLabel(const QString& label)
{
    m_label = label;
}

void ViewModelsDefaultFieldModel::SetSourceModel(QAbstractItemModel* model)
{
    if (m_sourceModel == model) {
        return;
    }
    
    beginResetModel();
    m_connections.Clear();
    m_sourceModel = model;
    static std::function<QModelIndex(QAbstractListModel*, const QModelIndex&)> mapIndex ([](QAbstractListModel* m, const QModelIndex& index){ return m->index(index.row() + 1, index.column()); });
    if(m_sourceModel != nullptr) {
        m_connections.connect(m_sourceModel, &QAbstractItemModel::dataChanged,
                              [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
        { emit dataChanged(mapIndex(this, topLeft), mapIndex(this, bottomRight), roles);});

        m_connections.connect(m_sourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
                              [this](const QModelIndex& index,int s,int e) {
                                    beginInsertRows(index, s + 1, e + 1);
                              });

        m_connections.connect(m_sourceModel, &QAbstractItemModel::rowsInserted,
                              [this]{ endInsertRows(); });

        m_connections.connect(m_sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                              [this](const QModelIndex& index,int s,int e) {
                                    beginRemoveRows(index, s + 1, e + 1);
                              });

        m_connections.connect(m_sourceModel, &QAbstractItemModel::rowsRemoved,
                              [this]{ endRemoveRows(); });

        m_connections.connect(m_sourceModel, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                              this, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));

        m_connections.connect(m_sourceModel, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                              this, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));

        m_connections.connect(m_sourceModel, &QAbstractItemModel::modelAboutToBeReset, [this]{ beginResetModel(); });
        m_connections.connect(m_sourceModel, &QAbstractItemModel::modelReset, [this]{ endResetModel(); });
    }
    endResetModel();
}

QVariant ViewModelsDefaultFieldModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    
    if(index.row() == 0) {
        if(role == Qt::DisplayRole) {
            return m_label;
        }
        return QVariant();
    }
    
    return m_sourceModel ? m_sourceModel->data(m_sourceModel->index(index.row() - 1, index.column()), role) : QVariant();
}

int ViewModelsDefaultFieldModel::rowCount(const QModelIndex& index) const
{
    return m_sourceModel ? m_sourceModel->rowCount(index) + 1 : 1;
}
