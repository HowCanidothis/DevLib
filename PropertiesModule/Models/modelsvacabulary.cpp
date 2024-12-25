#include "modelsvacabulary.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

ModelsVocabulary::ModelsVocabulary(const HeaderData& dictionary)
    : m_header(dictionary)
{
#ifdef UNITS_MODULE_LIB
    QSet<const Measurement*> measurements;
    for(const auto& value : ::make_const(m_header)) {
        Q_ASSERT(value.Label != nullptr);
        if(value.Measurement != nullptr) {
            measurements.insert(value.Measurement());
        }
    }
    for(const auto* measurement : measurements) {
        measurement->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{ UpdateUi([]{}); }).MakeSafe(m_connections);
    }

#endif
}

const QVariant& ModelsVocabulary::SelectValue(const Name& name, const QHash<Name, QVariant>& row)
{
    static QVariant result;
    auto foundIt = row.find(name);
    if(foundIt != row.end()) {
        return foundIt.value();
    }
    return result;
}

const ModelsVocabulary::HeaderDataValue& ModelsVocabulary::GetHeader(qint32 column) const
{
    static ModelsVocabulary::HeaderDataValue result = { Name(), TR_NONE, nullptr};
    if(column < 0 || column >= m_header.size()) {
        return result;
    }
    return m_header.at(column);
}
#ifdef WIDGETS_MODULE_LIB
#include <WidgetsModule/internal.hpp>

TViewModelsListBase<ModelsVocabulary>* ModelsVocabulary::CreateListModel(qint32 column, QObject* parent)
{
    return new TViewModelsListBase<ModelsVocabulary>(parent, [column](const SharedPointer<ModelsVocabulary>& ptr, const QModelIndex& index, qint32 role) -> QVariant {
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            if(index.row() == 0) {
                return QString();
            }

            const auto& header = ptr->GetHeader(column);
#ifdef UNITS_MODULE_LIB
            QVariant value = ptr->SelectValue(header.ColumnKey, ptr->At(index.row() - 1));
            if(value.isValid() && header.Measurement != nullptr) {
                value = header.Measurement()->GetCurrentUnit()->GetBaseToUnitConverter()(value.toDouble());
            }
            return value;
#else
            return ptr->SelectValue(header.ColumnKey, ptr->At(index.row() - 1));
#endif

        } else if(role == Qt::UserRole) {
            return QVariant(index.row() - 1);
        }
        return QVariant();
    }, [](const SharedPointer<ModelsVocabulary>& ptr){
        return ptr->GetSize() + 1;
    });
}
#endif
