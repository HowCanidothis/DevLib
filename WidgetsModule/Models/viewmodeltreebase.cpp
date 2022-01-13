#include "viewmodeltreebase.h"

#include "modelstreeitembase.h"

ViewModelsTreeBase::ViewModelsTreeBase(QObject* parent)
    : Super(parent)
    , m_errorIcon(IconsManager::GetInstance().GetIcon("ErrorIcon"))
    , m_warningIcon(IconsManager::GetInstance().GetIcon("WarningIcon"))
{
}

ViewModelsTreeBase::~ViewModelsTreeBase()
{
    if(m_data != nullptr) {
        m_data->OnAboutToBeDestroyed -= this;
        m_data->DisconnectModel(this);
    }
}

void ViewModelsTreeBase::SetData(const ModelsTreeWrapperPtr& data)
{
    beginResetModel();
    if(m_data != nullptr) {
        m_data->DisconnectModel(this);
    }
    m_data = data;
    if(m_data != nullptr) {
        m_data->ConnectModel(this);
        m_data->OnAboutToBeDestroyed += { this, [this] {
            m_data = nullptr;
        }};
    }
    endResetModel();
}

QModelIndex ViewModelsTreeBase::index(int row, int column, const QModelIndex& parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if(parent.isValid()) {
        auto* item = AsItem(parent)->GetChilds().at(row).get();
        return createIndex(row, column, item);
    }
    auto* item = m_data->GetRoot()->GetChilds().at(row).get();
    return createIndex(row, column, item);
}

QModelIndex ViewModelsTreeBase::parent(const QModelIndex& child) const
{
    auto* node = AsItem(child);
    if(node->GetParent() == m_data->GetRoot()) {
        return QModelIndex();
    } else if (node == m_data->GetRoot()) {
        return QModelIndex();
    }
    return createIndex(node->GetParentRow(), 0, node->GetParent());
}

int ViewModelsTreeBase::rowCount(const QModelIndex& parent) const
{
    if(m_data == nullptr) {
        return 0;
    }
    if(parent.isValid()) {
        auto* item = AsItem(parent);
        return item->GetChilds().size();
    }
    return m_data->GetRoot()->GetChilds().size();
}

int ViewModelsTreeBase::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant ViewModelsTreeBase::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
        auto* item = AsItem(index);
        return item->GetLabel();
    }
    case Qt::DecorationRole: {
        auto* item = AsItem(index);
        return item->GetIcon();
    }
    case Qt::FontRole: {
        auto* item = AsItem(index);
        return item->GetFont();
    }
    default:
        break;
    }

    return QVariant();
}

ModelsTreeItemBase* ViewModelsTreeBase::AsItem(const QModelIndex& index) const
{
    return (index.isValid() && index.internalId() != 0) ? reinterpret_cast<ModelsTreeItemBase*>(index.internalPointer()) : m_data->GetRoot();
}

QModelIndex ViewModelsTreeBase::AsIndex(class ModelsTreeItemBase* item) const {
    return createIndex(item->GetRow(), 0, item);
}
