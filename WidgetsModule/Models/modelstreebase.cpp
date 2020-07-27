#include "modelstreebase.h"

#include <SharedModule/External/external.hpp>

ModelsTreeBase::ModelsTreeBase()
{
    m_root = new ModelsTreeBaseDefaultItem();
}

void ModelsTreeBase::AddChild(const QModelIndex& parent, const SharedPointer<ModelsTreeBaseItem>& item)
{
    auto* parentItem = AsItem(parent);
    beginInsertRows(parent, parentItem->Childs.size(), parentItem->Childs.size());
    parentItem->AddChild(item);
    endInsertRows();
}

void ModelsTreeBase::Update(const std::function<ModelsTreeBaseItemPtr (const ModelsTreeBaseItemPtr&)>& resetFunction)
{
    emit layoutAboutToBeChanged();
    m_root = resetFunction(m_root);
    emit layoutChanged();
}

void ModelsTreeBase::remove(const QModelIndex& parent, const std::function<bool (const ModelsTreeBaseItem*)>& removePredicate)
{
    for(qint32 i(0); i < rowCount(parent); i++) {
        auto mi = index(i, 0, parent);
        auto* item = AsItem(mi);
        if(removePredicate(item)) {
            beginRemoveRows(parent, i, 0);
            AsItem(parent)->Childs.remove(i);
            endRemoveRows();
            i--;
        } else {
            remove(mi, removePredicate);
        }
    }
}

void ModelsTreeBase::Clear()
{
    beginResetModel();
    m_root->Childs.clear();
    endResetModel();
}

void ModelsTreeBase::ForeachChild(const QModelIndex& parent, const std::function<void (const ModelsTreeBaseItemPtr& item)>& predicate)
{
    forEachModelIndex(this, parent, [this, predicate](const QModelIndex& index) {
        predicate(AsItemPtr(index));
    });
}

void ModelsTreeBase::Remove(const std::function<bool (const ModelsTreeBaseItem*)>& removePredicate)
{
    remove(QModelIndex(), removePredicate);
}

QModelIndex ModelsTreeBase::index(int row, int column, const QModelIndex& parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if(parent.isValid()) {
        auto* item = AsItem(parent)->Childs.at(row).get();
        return createIndex(row, column, item);
    }
    auto* item = m_root->Childs.at(row).get();
    return createIndex(row, column, item);
}

QModelIndex ModelsTreeBase::parent(const QModelIndex& child) const
{
    auto* node = AsItem(child);
    if(node->Parent == m_root.get()) {
        return QModelIndex();
    }
    return createIndex(node->GetParentRow(), 0, node->Parent);
}

int ModelsTreeBase::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return AsItem(parent)->Childs.size();
    }
    return m_root->Childs.size();
}

int ModelsTreeBase::columnCount(const QModelIndex&) const
{
    return 1;
}

ModelsTreeBaseItem* ModelsTreeBase::AsItem(const QModelIndex& index) const
{
    return reinterpret_cast<ModelsTreeBaseItem*>(index.internalPointer());
}

ModelsTreeBaseItemPtr ModelsTreeBase::AsItemPtr(const QModelIndex& index) const
{
    auto* item = AsItem(index);
    if(item->Parent == nullptr) {
        return m_root;
    }
    const auto& childs = item->Parent->Childs;
    auto foundIt = std::find_if(childs.begin(), childs.end(), [item](const ModelsTreeBaseItemPtr& arrayItem) {
        return arrayItem.get() == item;
    });
    if(foundIt == childs.end()) {
        return m_root;
    }
    return *foundIt;
}

qint32 ModelsTreeBaseItem::GetRow() const
{
    if(Parent != nullptr) {
        auto foundIt = std::find_if(Parent->Childs.begin(), Parent->Childs.end(), [this](const SharedPointer<ModelsTreeBaseItem>& item){
            return item.get() == this;
        });
        return foundIt == Parent->Childs.end() ? -1 : std::distance(Parent->Childs.begin(), foundIt);
    }
    return 0;
}

static void foreachChild(const ModelsTreeBaseItem* parent, const std::function<void (ModelsTreeBaseItem* child)>& action)
{
    for(const auto& child : parent->Childs) {
        action(child.get());
        foreachChild(child.get(), action);
    }
}

void ModelsTreeBaseItem::ForeachChild(const std::function<void (ModelsTreeBaseItem* child)>& action) const
{
    foreachChild(this, action);
}

qint32 ModelsTreeBaseItem::GetParentRow() const
{
    if(Parent != nullptr) {
        return Parent->GetRow();
    }
    return 0;
}

void ModelsTreeBaseItem::AddChild(const SharedPointer<ModelsTreeBaseItem>& item)
{
    item->Parent = this;
    Childs.append(item);
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
