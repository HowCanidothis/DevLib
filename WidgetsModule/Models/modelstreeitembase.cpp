#include "modelstreeitembase.h"

ModelsTreeItemBase::ModelsTreeItemBase(ModelsTreeItemBase* parent)
    : m_parent(parent)
{
}

ModelsTreeItemBase::ModelsTreeItemBase(const ModelsTreeItemBase& o)
{
    m_parent = o.m_parent;
    m_childs = o.m_childs;
}

ModelsTreeItemBase& ModelsTreeItemBase::operator=(ModelsTreeItemBase& o)
{
    m_parent = o.m_parent;
    m_childs = o.m_childs;
    return *this;
}

void ModelsTreeItemBase::AddChild(const SharedPointer<ModelsTreeItemBase>& item)
{
    item->m_parent = this;
    m_childs.append(item);
}

void ModelsTreeItemBase::RemoveChild(qint32 i)
{
    m_childs.remove(i);
}

void ModelsTreeItemBase::RemoveChilds()
{
    m_childs.clear();
}

void ModelsTreeItemBase::foreachChild(ModelsTreeItemBase* item, const HandlerFunc& handler, const FilterFunc& filterFunc)
{
    for(const auto& child : item->GetChilds()) {
        handler(child.get());
        if (filterFunc(item)){
            foreachChild(child.get(), handler, filterFunc);
        }
    }
}

void ModelsTreeItemBase::foreachChildAfter(ModelsTreeItemBase* item, const HandlerFunc& handler, const FilterFunc& filterFunc)
{
    for(const auto& child : item->GetChilds()) {
        if (filterFunc(item)){
            foreachChildAfter(child.get(), handler, filterFunc);
        }
        handler(child.get());
    }
}

void ModelsTreeItemBase::ForeachChild(const HandlerFunc& handler, const FilterFunc& filterFunc) const
{
    foreachChild(const_cast<ModelsTreeItemBase*>(this), handler, filterFunc);
}

void ModelsTreeItemBase::ForeachChildAfter(const HandlerFunc& handler, const FilterFunc& filterFunc) const
{
    foreachChildAfter(const_cast<ModelsTreeItemBase*>(this), handler, filterFunc);
}

qint32 ModelsTreeItemBase::GetRow() const
{
    if(m_parent != nullptr) {
        const auto& childs = m_parent->GetChilds();
        auto foundIt = std::find_if(childs.begin(), childs.end(), [this](const SharedPointer<ModelsTreeItemBase>& item){
            return item.get() == this;
        });
        return foundIt == childs.end() ? -1 : std::distance(childs.begin(), foundIt);
    }
    return 0;
}

qint32 ModelsTreeItemBase::GetParentRow() const
{
    if(m_parent != nullptr) {
        return m_parent->GetRow();
    }
    return 0;
}

Qt::CheckState ModelsTreeItemBase::GetChecked(const qint64& key) const {
    auto iter = m_checkedMap.find(key);
    if (iter == m_checkedMap.end()) {
        iter = m_checkedMap.insert(key, Qt::Unchecked);
    }
    return iter.value();
}

void ModelsTreeItemBase::SetChecked(const qint64& key, Qt::CheckState value) {
    m_checkedMap[key] = value;
}

void ModelsTreeItemBase::clone(ModelsTreeItemBase* toItem) const
{
    toItem->m_childs = m_childs;
    toItem->m_parent = m_parent;
}

QVector<ModelsTreeItemBase*> ModelsTreeItemBase::GetPath() const
{
    QVector<ModelsTreeItemBase*> result;

    auto parent = GetParent();

    while(parent != nullptr) {
        result.prepend(parent);
        parent = parent->GetParent();
    }

    return result;
}
