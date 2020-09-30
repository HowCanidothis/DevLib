#include "modelstablebase.h"

ModelsTableBase::ModelsTableBase(QObject* parent)
    : Super(parent)
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
    m_data->ConnectModel(this);
    endResetModel();
}
