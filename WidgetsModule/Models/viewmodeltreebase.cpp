#include "viewmodeltreebase.h"

#include "modelstreeitembase.h"

ViewModelTreeBase::ViewModelTreeBase()
{
}

ViewModelTreeBase::~ViewModelTreeBase()
{
    if(m_data != nullptr) {
        m_data->OnAboutToBeDestroyed -= this;
    } else {
        m_data->DisconnectModel(this);
    }
}

void ViewModelTreeBase::SetData(const ModelsTreeWrapperPtr& data)
{
    beginResetModel();
    if(m_data != nullptr) {
        m_data->DisconnectModel(this);
    }
    m_data = data;
    m_data->ConnectModel(this);
    m_data->OnAboutToBeDestroyed += { this, [this] {
        m_data = nullptr;
    }};
    endResetModel();
}

QModelIndex ViewModelTreeBase::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex ViewModelTreeBase::parent(const QModelIndex& child) const
{
    auto* node = AsItem(child);
    if(node->GetParent() == m_data->GetRoot()) {
        return QModelIndex();
    }
    return createIndex(node->GetParentRow(), 0, node->GetParent());
}

int ViewModelTreeBase::rowCount(const QModelIndex& parent) const
{
    if(m_data == nullptr) {
        return 0;
    }
    if(parent.isValid()) {
        return AsItem(parent)->GetChilds().size();
    }
    return m_data->GetRoot()->GetChilds().size();
}

int ViewModelTreeBase::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant ViewModelTreeBase::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        auto* item = AsItem(index);
        return item->GetLabel();
    }
    case Qt::DecorationRole: {
        auto* item = AsItem(index);
        return item->GetIcon();
    }
    /*case Qt::FontRole: {
        auto* item = AsItem(index);
        if(item->Parent == m_root.get() && item->ComputingWell != nullptr && item->ComputingWell.get() == m_activeWell) {
            QFont font;
            font.setBold(true);
            return font;
        }
        break;
    }*/
    default:
        break;
    }

    return QVariant();
}

ModelsTreeItemBase* ViewModelTreeBase::AsItem(const QModelIndex& index) const
{
    return index.isValid() ? reinterpret_cast<ModelsTreeItemBase*>(index.internalPointer()) : m_data->GetRoot();
}
