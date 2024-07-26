#ifndef MODELSTREE_H
#define MODELSTREE_H

#include <SharedModule/internal.hpp>

#include "modelstreeitembase.h"

using ModelsTreeItemBasePtr = SharedPointer<class ModelsTreeItemBase>;

class ModelsTreeItemBase;

class ModelsTree : public ModelsTreeWrapper
{
public:
    using value_type = ModelsTreeItemBase*;

    ModelsTree(const SharedPointer<ModelsTreeItemBase>& root = SharedPointer<ModelsTreeItemBase>());

    void SetRoot(const SharedPointer<ModelsTreeItemBase>& root);

    value_type At(qint32 index) const;
    void Clear();
    void Insert(qint32 before, qint32 count, const std::function<SharedPointer<ModelsTreeItemBase> ()>& itemCreator);
    const ModelsTreeItemBasePtr& Insert(qint32 before, const SharedPointer<ModelsTreeItemBase>& item, ModelsTreeItemBase* parent);
    const ModelsTreeItemBasePtr& Add(const SharedPointer<ModelsTreeItemBase>& item, ModelsTreeItemBase* parent);
    void Edit(ModelsTreeItemBase* item, const FAction& action, const QVector<qint32>& roles);
    bool EditWithCheck(qint32 index, const std::function<FAction (ModelsTreeItemBase*)>& action, const QVector<qint32>& roles = QVector<qint32>());
    bool EditWithCheck(ModelsTreeItemBase* item, const std::function<FAction (ModelsTreeItemBase*)>& action, const QVector<qint32>& roles = QVector<qint32>());
    void ForeachChangeValue(const std::function<bool (ModelsTreeItemBase* item)>& handler);
    void Update(const std::function<void ()>& predicate);
    void Change(const std::function<void ()>& predicate);
    void Remove(const QSet<qint32>& indexes);
    void Remove(ModelsTreeItemBase* item);
    void Remove(const QSet<ModelsTreeItemBase*>& items);
    void Remove(const std::function<bool (ModelsTreeItemBase*)>& predicate) { remove(m_root.get(), predicate); }
    void RemoveChildren(ModelsTreeItemBase* item);
    void SetCheckedRecursive(qint64 key, ModelsTreeItemBase* item, Qt::CheckState checked, const ModelsTreeItemBase::FilterFunc& filter);
    void SetChecked(qint64 key, ModelsTreeItemBase* item, Qt::CheckState checked);
    qint32 GetSize() const { return m_root->GetChilds().size(); }
    bool IsEmpty() const { return GetSize() == 0; }

    bool HasIndex(qint32 index) const { return index >= 0 && index < m_root->GetChilds().size(); }

    const SharedPointer<ModelsTreeItemBase>& GetRootPtr() const { return m_root; }

    ModelsTreeItemBase* GetRoot() override { return m_root.get(); }

    const SharedPointer<ModelsTreeItemBase>& ItemPtrFromPointer(ModelsTreeItemBase* item) const;
    ModelsTreeItemBase* ItemFromModelIndex(const QModelIndex& modelIndex);
    const SharedPointer<ModelsTreeItemBase>& ItemPtrFromModelIndex(const QModelIndex& modelIndex);

private:
    void remove(ModelsTreeItemBase* parent, const std::function<bool (ModelsTreeItemBase*)>& removePredicate);
    void removeChilds(ModelsTreeItemBase* parent, const QSet<ModelsTreeItemBase*> items);

private:
    SharedPointer<ModelsTreeItemBase> m_root;
};

using ModelsTreePtr = SharedPointer<ModelsTree>;

#endif // MODELSTREE_H
