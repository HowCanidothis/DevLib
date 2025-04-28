#ifndef VIEWMODELSDEFAULTFIELDMODEL_H
#define VIEWMODELSDEFAULTFIELDMODEL_H

#include <QAbstractListModel>
#include <QAbstractItemModel>
#include <SharedModule/internal.hpp>

class ViewModelsDefaultFieldModel : public QAbstractListModel
{
    using Super = QAbstractListModel;
public:
    ViewModelsDefaultFieldModel(QObject* parent, const FTranslationHandler& label = nullptr, QAbstractItemModel* model  = nullptr);

    ViewModelsDefaultFieldModel& SetDefaultFieldLabel(const FTranslationHandler& label);
    ViewModelsDefaultFieldModel& SetSourceModel(QAbstractItemModel* model);

    QAbstractItemModel* GetSourceModel() const { return m_sourceModel; }

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;

private:
    FTranslationHandler m_label;
    QAbstractItemModel* m_sourceModel;
    QtLambdaConnections m_connections;
};

#endif // ViewModelsDefaultFieldModel_H
