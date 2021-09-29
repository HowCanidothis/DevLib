#include "modelstablebase.h"

ModelsTableBase::ModelsTableBase(QObject* parent)
    : Super(parent)
    , m_errorIcon(IconsManager::GetInstance().GetIcon("ErrorIcon"))
    , m_warningIcon(IconsManager::GetInstance().GetIcon("WarningIcon"))
    , m_infoIcon(IconsManager::GetInstance().GetIcon("InfoIcon"))
{

}

ModelsTableBase::~ModelsTableBase()
{
    if(m_data != nullptr) {
        m_data->DisconnectModel(this);
    }
}

void ModelsTableBase::SetData(const ModelsTableWrapperPtr& data)
{
    beginResetModel();
    if(m_data != nullptr) {
        m_data->DisconnectModel(this);
    }
    m_data = data;
    if(m_data != nullptr) {
        m_data->ConnectModel(this);
    }
    endResetModel();
}
