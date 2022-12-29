#include "viewmodelvacabulary.h"
#include "modelsfiltermodelbase.h"
#include "modelslistbase.h"

ModelsVocabularyViewModel::ModelsVocabularyViewModel(QObject* parent)
    : Super(parent)
{
    setProperty(WidgetProperties::ExtraFieldsCount, 1);

    auto editRoleHandler = [this](qint32 row, qint32 column) -> QVariant {
        auto ret = GetData()->At(row)[GetData()->GetHeader(column).ColumnKey];
        auto foundIt = GetterDelegates.find(column);
        if(foundIt != GetterDelegates.end()) {
            foundIt.value()(ret);
        }
        return ret;
    };

    auto displayRoleHandlers = [this](qint32 row, qint32 column) -> QVariant {
        auto ret = GetData()->At(row)[GetData()->GetHeader(column).ColumnKey];
        auto foundIt = GetterDisplayDelegates.find(column);
        if(foundIt != GetterDisplayDelegates.end()) {
            foundIt.value()(ret);
        }
        return ret;
    };

    m_roleDataHandlers.insert(Qt::EditRole, editRoleHandler);
    m_roleDataHandlers.insert(Qt::DisplayRole, displayRoleHandlers);

    m_roleHorizontalHeaderDataHandlers.insert(Qt::DisplayRole, [this](qint32 section){
        return GetData()->GetHeader(section).Label->Native();
    });

    m_roleSetDataHandlers.insert(Qt::EditRole, [this](qint32 row, qint32 column, const QVariant& value) -> bool {
        QVariant concreteValue = value;
        auto foundIt = SetterDelegates.find(column);
        if(foundIt != SetterDelegates.end()) {
            foundIt.value()(concreteValue);
        }
        return GetData()->EditWithCheck(row, [&](QHash<Name, QVariant>& row) -> FAction {
            const auto& key = GetData()->GetHeader(column).ColumnKey;
            if(row[key] == concreteValue) {
                return nullptr;
            }
            return [&]{ row.insert(key, concreteValue); };
        }, { column });
});
}

bool ModelsVocabularyViewModel::setData(const QModelIndex& index, const QVariant& value, qint32 role)
{
    if(!index.isValid()) {
        return false;
    }

    if(isLastEditRow(index)) {
        if(role == Qt::EditRole) {
            QHash<Name, QVariant> newRow;
            GetData()->Append(newRow);
        } else {
            return false;
        }
    }

    return Super::setData(index, value, role);
}

QVariant ModelsVocabularyViewModel::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    if(isLastEditRow(index)) {
        return QVariant();
    }

    return Super::data(index, role);
}

qint32 ModelsVocabularyViewModel::rowCount(const QModelIndex&) const
{
    return GetData() == nullptr ? 0 : (GetData()->GetSize() + 1);
}

qint32 ModelsVocabularyViewModel::columnCount(const QModelIndex&) const
{
    return GetData() != nullptr ? GetData()->GetColumnsCount() : 0;
}

Qt::ItemFlags ModelsVocabularyViewModel::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
