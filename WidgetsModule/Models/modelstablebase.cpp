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

QVariant ModelsTableBase::data(const QModelIndex& index, qint32 role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    auto foundIt = m_roleDataHandlers.find(role);
    if(foundIt == m_roleDataHandlers.end()) {
        return QVariant();
    }
    return foundIt.value()(index.row(), index.column());
}

bool ModelsTableBase::setData(const QModelIndex& index, const QVariant& data, qint32 role)
{
    if(!index.isValid()) {
        return false;
    }
    auto foundIt = m_roleSetDataHandlers.find(role);
    if(foundIt == m_roleSetDataHandlers.end()) {
        return false;
    }
    return foundIt.value()(index.row(), index.column(), data);
}

QVariant ModelsTableBase::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const
{
    if(orientation == Qt::Horizontal) {
        auto foundIt = m_roleHorizontalHeaderDataHandlers.find(role);
        if(foundIt == m_roleHorizontalHeaderDataHandlers.end()) {
            return QVariant();
        }
        return foundIt.value()(section);
    } else {
        auto foundIt = m_roleVerticalHeaderDataHandlers.find(role);
        if(foundIt == m_roleVerticalHeaderDataHandlers.end()) {
            return QVariant();
        }
        return foundIt.value()(section);
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

    OnModelChanged();
}

ModelsIconsContext::ModelsIconsContext()
    : ErrorIcon(IconsManager::GetInstance().GetIcon("ErrorIcon"))
    , WarningIcon(IconsManager::GetInstance().GetIcon("WarningIcon"))
    , InfoIcon(IconsManager::GetInstance().GetIcon("InfoIcon"))
{

}
