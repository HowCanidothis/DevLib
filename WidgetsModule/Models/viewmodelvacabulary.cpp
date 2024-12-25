#include "viewmodelvacabulary.h"
#include "modelsfiltermodelbase.h"
#include "modelslistbase.h"
#include "UnitsModule/internal.hpp"

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
        const auto& header = GetData()->GetHeader(section);
        auto result = header.Label();
        if(header.Measurement != nullptr && header.Measurement() != nullptr){
            result = MeasurementManager::MakeMeasurementString(result, header.Measurement());
        }
        return result;
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

qint32 ModelsVocabularyViewModel::columnCount(const QModelIndex&) const
{
    return GetData() != nullptr ? GetData()->GetColumnsCount() : 0;
}

Qt::ItemFlags ModelsVocabularyViewModel::flags(const QModelIndex& index) const
{
    if(IsLastRow(index)) {
        return Super::flags(index);
    }
    return StandardEditableFlags();
}
