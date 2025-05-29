#include "viewmodelsdefaultcomponents.h"

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

TViewModelsColumnComponentsBuilderBase& TViewModelsColumnComponentsBuilderBase::AddDefaultColors(LocalPropertyColor* enabledCellColor, LocalPropertyColor* disabledCellColor,
                               LocalPropertyColor* enabledTextColor, LocalPropertyColor* disabledTextColor)
{
    auto* model = m_viewModel;
    m_viewModel->ColumnComponents.AddDefaultComponent(Qt::BackgroundRole, ViewModelsTableColumnComponents::ColumnComponentData()
                                         .SetGetter([model, enabledCellColor, disabledCellColor](const QModelIndex& index) {
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
