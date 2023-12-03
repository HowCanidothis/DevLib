#include "modelslistbase.h"


QVariant ViewModelsStandardListModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || GetData() == nullptr) {
        return QVariant();
    }

    return GetData()->At(index.row()).value(role, QVariant());
}

int ViewModelsStandardListModel::columnCount(const QModelIndex& index) const
{
    if(index.isValid()) {
        return 0;
    }
    return 1;
}

void ModelsStandardListModel::fillContainerWithEnum(container_type& container, const QStringList& names, qint32 startsWith, qint32 endsWith)
{
    for(qint32 i(startsWith), e(endsWith); i <= e; i++) {
        ModelsStandardListModel::value_type data;
        const auto& label = names.at(i);
        data.insert(Qt::DisplayRole, label);
        data.insert(Qt::EditRole, label);
        data.insert(Qt::ToolTipRole, label);
        data.insert(IdRole, i);
        container.append(data);
    }
}
