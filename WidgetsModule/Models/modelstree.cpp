#include "modelstree.h"

#include "modelstreeitembase.h"

void ModelsTree::remove(ModelsTreeItemBase* parent, const std::function<bool (ModelsTreeItemBase*)>& removePredicate)
{
    QSet<ModelsTreeItemBase*> toRemove;
    auto& childs = parent->m_childs;
    for(const auto& child : childs) {
        if(removePredicate(child.get())) {
            toRemove.insert(child.get());
        } else {
            remove(child.get(), removePredicate);
        }
    }

    removeChilds(parent, toRemove);
}

void ModelsTree::removeChilds(ModelsTreeItemBase* parent, const QSet<ModelsTreeItemBase*> toRemove)
{
    if(!toRemove.isEmpty() && parent != nullptr) {
        auto& childs = parent->m_childs;
        struct RemoveGroup
        {
            qint32 Since = -2;
            qint32 To = -2;
        };

        QVector<RemoveGroup> removeGroups(1);
        RemoveGroup* currentGroup = &removeGroups.last();
        qint32 i(0);
        bool found = false;
        for(const auto& child : childs) {
            if(toRemove.contains(child.get())) {
                if(currentGroup->Since < 0) {
                    found = true;
                    currentGroup->Since = i;
                } else {
                    if(currentGroup->To + 1 != i) {
                        removeGroups.append(RemoveGroup());
                        currentGroup = &removeGroups.last();
                        currentGroup->Since = i;
                    }
                }
                currentGroup->To = i;
            }
            i++;
        }

        if(found) {
            for(const auto& removeGroup : adapters::reverse(removeGroups)) {
                OnAboutToRemoveRows(removeGroup.Since, removeGroup.To, parent);
                childs.remove(removeGroup.Since, removeGroup.To - removeGroup.Since + 1);
                OnRowsRemoved();
            }
        }
    }
}

void ModelsTree::ForeachChangeValue(const std::function<bool (ModelsTreeItemBase* item)>& handler)
{
    m_root->ForeachChild([handler, this](ModelsTreeItemBase* item){
        if(handler(item)) {
            OnTreeValueChanged(0, item, {});
        }
    });
}

void ModelsTree::Update(const std::function<void ()>& predicate)
{
    OnAboutToBeUpdated();
    predicate();
    OnUpdated();
}

ModelsTree::ModelsTree(ModelsTreeItemBase* root)
{
    m_root = root;
}

void ModelsTree::SetRoot(const ModelsTreeItemBasePtr& root)
{
    OnAboutToBeReseted();
    m_root = root;
    OnReseted();
}

void ModelsTree::Clear()
{
    OnAboutToBeReseted();
    m_root->m_childs.clear();
    OnReseted();
}

const ModelsTreeItemBasePtr& ModelsTree::Add(const ModelsTreeItemBasePtr& item, ModelsTreeItemBase* parent)
{
    OnAboutToInsertRows(parent->GetChilds().size(), parent->GetChilds().size(), parent);
    parent->AddChild(item);
    OnRowsInserted();
    return item;
}

void ModelsTree::Change(const std::function<void ()>& predicate)
{
    OnAboutToBeReseted();
    predicate();
    OnReseted();
}

void ModelsTree::Remove(ModelsTreeItemBase* item)
{
    if(item == m_root.get()) {
        return;
    }

    removeChilds(item->GetParent(), {item});
}

void ModelsTree::RemoveChildren(ModelsTreeItemBase* item)
{
    if (item->m_childs.isEmpty()) {
        return ;
    }
    OnAboutToRemoveRows(0, item->m_childs.size()-1, item);
    item->m_childs.clear();
    OnRowsRemoved();
}

void ModelsTree::SetCheckedRecursive(qint64 key, ModelsTreeItemBase* item, Qt::CheckState checkState, const ModelsTreeItemBase::FilterFunc& funcFilter)
{
    if(item->IsChecked(key) == checkState) {
        return;
    }

    SetChecked(key, item, checkState);

    if(funcFilter != nullptr) {
        item->ForeachChild([checkState, this, &key](ModelsTreeItemBase* item){
            SetChecked(key, item, checkState);
        }, funcFilter);
    }

    auto parents = item->GetPath();

    for(auto* parent : adapters::reverse(parents)) {
        Qt::CheckState rootCheckState = Qt::Unchecked;
        bool hasUnChecked = false;
        bool hasChecked = false;
        for(const auto& child : parent->GetChilds()) {
            if(rootCheckState == Qt::PartiallyChecked) {
                break;
            }
            switch(child->IsChecked(key)) {
            case Qt::PartiallyChecked:
                rootCheckState = Qt::PartiallyChecked;
                break;
            case Qt::Checked:
                if(hasUnChecked) {
                    rootCheckState = Qt::PartiallyChecked;
                } else {
                    hasChecked = true;
                }
                break;
            case Qt::Unchecked:
                if(hasChecked) {
                    rootCheckState = Qt::PartiallyChecked;
                } else {
                    hasUnChecked = true;
                }
                break;
            default: break;
            };
        }
        if(rootCheckState != Qt::PartiallyChecked) {
            if(hasChecked) {
                rootCheckState = Qt::Checked;
            } else {
                rootCheckState = Qt::Unchecked;
            }
        }
        SetChecked(key, parent, rootCheckState);
    }
}

void ModelsTree::SetChecked(qint64 key, ModelsTreeItemBase* item, Qt::CheckState checked) {
    if (item->IsChecked(key) == checked) {
        return;
    }

    item->SetChecked(key, checked);
    OnTreeValueChanged(key, item, {Qt::CheckStateRole});
//    OnChanged();
}

const SharedPointer<ModelsTreeItemBase>& ModelsTree::ItemPtrFromPointer(ModelsTreeItemBase* item) const
{
    auto* parent = item->GetParent();
    if(parent == nullptr) {
        return m_root;
    }
    return parent->GetChildPtr(item);
}

ModelsTreeItemBase* ModelsTree::ItemFromModelIndex(const QModelIndex& modelIndex)
{
    if(!modelIndex.isValid()) {
        return m_root.get();
    }
    return reinterpret_cast<ModelsTreeItemBase*>(modelIndex.internalPointer());
}

const SharedPointer<ModelsTreeItemBase>& ModelsTree::ItemPtrFromModelIndex(const QModelIndex& modelIndex)
{
    if(!modelIndex.isValid()) {
        return m_root;
    }
    auto item = reinterpret_cast<ModelsTreeItemBase*>(modelIndex.internalPointer());
    Q_ASSERT(item->GetParent() != nullptr);
    return item->GetParent()->GetChildPtr(item);
}
