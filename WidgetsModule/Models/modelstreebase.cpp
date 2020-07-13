#include "modelstreebase.h"

ModelsTreeBase::ModelsTreeBase()
{
    m_root = new ModelsTreeBaseDefaultItem();
}

void ModelsTreeBase::AddChild(const QModelIndex& parent, ModelsTreeBaseItem* item)
{
    auto* parentItem = AsItem(parent);
    beginInsertRows(parent, parentItem->Childs.Size(), parentItem->Childs.Size());
    parentItem->AddChild(item);
    endInsertRows();
}

void ModelsTreeBase::Update(const std::function<ModelsTreeBaseItem* (ModelsTreeBaseItem*)>& resetFunction)
{
    beginResetModel();
    m_root = resetFunction(m_root.get());
    endResetModel();
}

QModelIndex ModelsTreeBase::index(int row, int column, const QModelIndex& parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if(parent.isValid()) {
        auto* item = AsItem(parent)->Childs.At(row);
        return createIndex(row, column, item);
    }
    auto* item = m_root->Childs.At(row);
    return createIndex(row, column, item);
}

QModelIndex ModelsTreeBase::parent(const QModelIndex& child) const
{
    auto* node = AsItem(child);
    if(node->Parent == m_root.data()) {
        return QModelIndex();
    }
    return createIndex(node->GetParentRow(), 0, node->Parent);
}

int ModelsTreeBase::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return AsItem(parent)->Childs.Size();
    }
    return m_root->Childs.Size();
}

int ModelsTreeBase::columnCount(const QModelIndex&) const
{
    return 1;
}

ModelsTreeBaseItem* ModelsTreeBase::AsItem(const QModelIndex& index) const
{
    return reinterpret_cast<ModelsTreeBaseItem*>(index.internalPointer());
}

qint32 ModelsTreeBaseItem::GetRow() const
{
    if(Parent != nullptr) {
        return Parent->Childs.IndexOf(const_cast<ModelsTreeBaseItem*>(this));
    }
    return 0;
}

qint32 ModelsTreeBaseItem::GetParentRow() const
{
    if(Parent != nullptr) {
        return Parent->GetRow();
    }
    return 0;
}

void ModelsTreeBaseItem::AddChild(ModelsTreeBaseItem* item)
{
    item->Parent = this;
    Childs.Append(item);
}

QVariant ModelsTreeBase::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        auto* item = AsItem(index);
        return item->GetLabel();
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
