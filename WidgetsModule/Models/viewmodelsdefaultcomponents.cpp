#include "viewmodelsdefaultcomponents.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

TViewModelsColumnComponentsBuilderBase& TViewModelsColumnComponentsBuilderBase::AddDefaultColors(LocalPropertyColor* enabledCellColor, LocalPropertyColor* disabledCellColor,
                               LocalPropertyColor* enabledTextColor, LocalPropertyColor* disabledTextColor, LocalPropertyColor* enabledCellColorAlt, LocalPropertyColor* disabledCellColorAlt)
{
    auto* model = m_viewModel;
    m_viewModel->ColumnComponents.AddDefaultComponent(BackgroundAltRole, ViewModelsTableColumnComponents::ColumnComponentData()
                                         .SetGetter([model, enabledCellColorAlt, disabledCellColorAlt](const QModelIndex& index) {
                                            switch(index.data(FieldHasErrorRole).toInt()) {
                                            case 1: return SharedSettings::GetInstance().StyleSettings.ErrorCellAltColor.Native();
                                            case 2: return SharedSettings::GetInstance().StyleSettings.WarningCellAltColor.Native();
                                            default: break;
                                            }
                                            if(model->flags(index).testFlag(Qt::ItemIsEditable)) {
                                                return enabledCellColorAlt->Native();
                                            }
                                            return disabledCellColorAlt->Native();
                                         }));
    m_viewModel->ColumnComponents.AddDefaultComponent(Qt::BackgroundRole, ViewModelsTableColumnComponents::ColumnComponentData()
                                         .SetGetter([model, enabledCellColor, disabledCellColor](const QModelIndex& index) {
                                            switch(index.data(FieldHasErrorRole).toInt()) {
                                            case 1: return SharedSettings::GetInstance().StyleSettings.ErrorCellColor.Native();
                                            case 2: return SharedSettings::GetInstance().StyleSettings.WarningCellColor.Native();
                                            default: break;
                                            }
                                            if(model->flags(index).testFlag(Qt::ItemIsEditable)) {
                                                return enabledCellColor->Native();
                                            }
                                            return disabledCellColor->Native();
                                         }));
    m_viewModel->ColumnComponents.AddDefaultComponent(Qt::TextColorRole, ViewModelsTableColumnComponents::ColumnComponentData()
                                         .SetGetter([model, enabledTextColor, disabledTextColor](const QModelIndex& index) {
                                            if(model->flags(index).testFlag(Qt::ItemIsEditable)) {
                                                return enabledTextColor->Native();
                                            }
                                            return disabledTextColor->Native();
                                         }));
    return *this;
}
