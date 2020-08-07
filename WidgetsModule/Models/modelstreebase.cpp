#include "modelstreebase.h"

#include <SharedModule/External/external.hpp>

ModelsTreeBase::ModelsTreeBase()
{
    m_root = new ModelsTreeBaseDefaultItem();
}

QModelIndex ModelsTreeBase::AddChild(const ModelsTreeBaseItemPtr& parent, const ModelsTreeBaseItemPtr& item)
{
    auto parentIndex = Find([&parent](const ModelsTreeBaseItem* node) {
        return node == parent.get();
    });

    return AddChild(parentIndex, item);
}

QModelIndex ModelsTreeBase::AddChild(const QModelIndex& parent, const SharedPointer<ModelsTreeBaseItem>& item)
{
    auto* parentItem = AsItem(parent);
    const auto& childs = parentItem->GetChilds();
    beginInsertRows(parent, childs.size(), childs.size());
    parentItem->AddChild(item);
    endInsertRows();
    return index(childs.size() - 1, 0, parent);
}

void ModelsTreeBase::findInternal(const QModelIndex& parent, const std::function<bool (const ModelsTreeBaseItem* )>& predicate, QModelIndex& result) const
{
    auto rows = rowCount(parent);
    for(qint32 i(0); i < rows; i++) {
        if(result.isValid()) {
            return;
        }
        auto mi = index(i, 0, parent);
        auto* item = AsItem(mi);
        if(predicate(item)) {
            result = mi;
            return;
        }

        findInternal(mi, predicate, result);
    }
}


QModelIndex ModelsTreeBase::Find(const std::function<bool (const ModelsTreeBaseItem* )>& predicate) const
{
    QModelIndex result;
    findInternal(QModelIndex(), predicate, result);
    return result;
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
            beginRemoveRows(parent, i, i);
            AsItem(parent)->RemoveChild(i);
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
    m_root->RemoveChilds();
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
        auto* item = AsItem(parent)->GetChilds().at(row).get();
        return createIndex(row, column, item);
    }
    auto* item = m_root->GetChilds().at(row).get();
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

QVector<QModelIndex> ModelsTreeBase::GetPath(const ModelsTreeBaseItem* item) const
{
    auto targetIndex = Find([item](const ModelsTreeBaseItem* child){
        return item == child;
    });

    QVector<QModelIndex> result;

    auto parentIndex = parent(targetIndex);

    while(parentIndex.isValid()) {
         result.prepend(parentIndex);
         parentIndex = parent(parentIndex);
    }

    return result;
}

int ModelsTreeBase::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return AsItem(parent)->GetChilds().size();
    }
    return m_root->GetChilds().size();
}

int ModelsTreeBase::columnCount(const QModelIndex&) const
{
    return 1;
}

ModelsTreeBaseItem* ModelsTreeBase::AsItem(const QModelIndex& index) const
{
    return index.isValid() ? reinterpret_cast<ModelsTreeBaseItem*>(index.internalPointer()) : m_root.get();
}

void ModelsTreeBaseItem::clone(ModelsTreeBaseItem* toItem) const
{
    toItem->m_childs = m_childs;
    toItem->Parent = Parent;
}

const ModelsTreeBaseItemPtr& ModelsTreeBase::AsItemPtr(const QModelIndex& index) const
{
    auto* item = AsItem(index);
    if(item->Parent == nullptr) {
        return m_root;
    }
    const auto& childs = item->Parent->GetChilds();
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
        const auto& childs = Parent->GetChilds();
        auto foundIt = std::find_if(childs.begin(), childs.end(), [this](const SharedPointer<ModelsTreeBaseItem>& item){
            return item.get() == this;
        });
        return foundIt == childs.end() ? -1 : std::distance(childs.begin(), foundIt);
    }
    return 0;
}

static void foreachChild(const ModelsTreeBaseItem* parent, const std::function<void (ModelsTreeBaseItem* child)>& action)
{
    for(const auto& child : parent->GetChilds()) {
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
    m_childs.append(item);
}

void ModelsTreeBaseItem::RemoveChilds()
{
    m_childs.clear();
}

void ModelsTreeBaseItem::RemoveChild(qint32 i)
{
    m_childs.remove(i);
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
