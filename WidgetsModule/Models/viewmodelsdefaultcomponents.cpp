#include "viewmodelsdefaultcomponents.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

void ViewModelsDefaultComponents::ApplyDefaultColors(LocalPropertyColor* enabledCellColor, LocalPropertyColor* disabledCellColor,
                               LocalPropertyColor* enabledTextColor, LocalPropertyColor* disabledTextColor)
{
    m_model->ColumnComponents.AddComponent(Qt::BackgroundRole, -1, ViewModelsTableColumnComponents::ColumnComponentData()
                                         .SetGetter([this, enabledCellColor, disabledCellColor](const QModelIndex& index, bool&) {
                                            if(m_model->flags(index).testFlag(Qt::ItemIsEditable)) {
                                                return enabledCellColor->Native();
                                            }
                                            return disabledCellColor->Native();
                                         }));
    m_model->ColumnComponents.AddComponent(Qt::TextColorRole, -1, ViewModelsTableColumnComponents::ColumnComponentData()
                                         .SetGetter([this, enabledTextColor, disabledTextColor](const QModelIndex& index, bool&) {
                                            if(m_model->flags(index).testFlag(Qt::ItemIsEditable)) {
                                                return enabledTextColor->Native();
                                            }
                                            return disabledTextColor->Native();
                                         }));
}
#ifdef UNITS_MODULE_LIB
void ViewModelsDefaultComponents::ApplyUnitsComponent(const ApplyUnitsComponentParams& map)
{
    for(const auto& [measurementName, columnGetters] : map) {
        auto* pMeasurement = MeasurementManager::GetInstance().GetMeasurement(measurementName).get();
        for(auto it(columnGetters.cbegin()), e(columnGetters.cend()); it != e; it++) {
            auto valueGetter = it.value();
            m_model->ColumnComponents.AddComponent(Qt::DisplayRole, it.key(), ViewModelsTableColumnComponents::ColumnComponentData()
            .SetGetter([valueGetter, pMeasurement](const QModelIndex& index, bool&) -> QString {
                bool hasValue = true;
                auto value = valueGetter.ValueGetter(index.row(), hasValue);
                if(!hasValue) {
                    return "-";
                }
                return QString::number(pMeasurement->BaseValueToCurrentUnit(value), 'f', pMeasurement->CurrentPrecision);
            })
            .SetHeader([valueGetter, pMeasurement](bool&){
                return valueGetter.Header().arg(pMeasurement->CurrentUnitLabel);
            }));
            m_model->ColumnComponents.AddComponent(Qt::EditRole, it.key(), ViewModelsTableColumnComponents::ColumnComponentData()
            .SetGetter([valueGetter, pMeasurement](const QModelIndex& index, bool&){
                bool hasValue = true;
                auto value = valueGetter.ValueGetter(index.row(), hasValue);
                return pMeasurement->BaseValueToCurrentUnit(value);
            })
            .SetSetter([valueGetter, pMeasurement](const QModelIndex& index, const QVariant& data, bool&){
                return valueGetter.ValueSetter(index.row(), pMeasurement->CurrentUnitToBaseValue(data.toDouble()));
            })
            );
        }
        m_model->AttachDependence(&pMeasurement->OnChanged, columnGetters.firstKey(), columnGetters.lastKey());
    }
}
#endif
