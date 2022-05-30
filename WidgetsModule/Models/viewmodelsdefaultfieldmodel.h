#ifndef VIEWMODELSDEFAULTFIELDMODEL_H
#define VIEWMODELSDEFAULTFIELDMODEL_H

#include <QAbstractListModel>
#include <QAbstractItemModel>
#include <SharedModule/internal.hpp>

class ViewModelsDefaultFieldModel : public QAbstractListModel
{
    using Super = QAbstractListModel;
public:
    ViewModelsDefaultFieldModel(QObject* parent);

    void SetDefaultFieldLabel(const QString& label);
    void SetSourceModel(QAbstractItemModel* model);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;

private:
    QString m_label;
    QAbstractItemModel* m_sourceModel;
    QtLambdaConnections m_connections;
};

#endif // ViewModelsDefaultFieldModel_H
