#include "progressbar.h"

ProgressBar::ProgressBar(const Name& processId, QProgressBar* parent)
    : Super(parent)
{
    parent->hide();

    ProcessFactory::Instance().OnDeterminate.Connect(CONNECTION_DEBUG_LOCATION, [parent, processId](size_t, const DescProcessDeterminateValueState& value){
        if(!processId.IsNull() && value.Id != processId) {
            return;
        }
        parent->setVisible(value.IsShouldStayVisible());
        parent->setMaximum(value.StepsCount);
        parent->setValue(value.CurrentStep);
        parent->setToolTip(value.Title);
    }).MakeSafe(m_connections);

    ProcessFactory::Instance().OnIndeterminate.Connect(CONNECTION_DEBUG_LOCATION, [this, parent, processId](size_t, const DescProcessValueState& value){
        if(!processId.IsNull() && value.Id != processId) {
            return;
        }
        bool visible = value.IsShouldStayVisible();
        if(!visible) {
            --m_counter;
        } else {
            ++m_counter;
        }
        parent->setVisible(m_counter > 0);
        parent->setMaximum(0);
        parent->setToolTip(value.Title);
    }).MakeSafe(m_connections);
}
