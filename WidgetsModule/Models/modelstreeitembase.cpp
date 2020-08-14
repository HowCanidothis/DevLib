#include "modelstreeitembase.h"

ModelsTreeItemBase::ModelsTreeItemBase(ModelsTreeItemBase* parent)
    : m_parent(parent)
{}

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

void ModelsTreeItemBase::foreachChild(ModelsTreeItemBase* item, const std::function<void (ModelsTreeItemBase*)>& handler)
{
    for(const auto& child : item->GetChilds()) {
        handler(child.get());
        foreachChild(child.get(), handler);
    }
}

void ModelsTreeItemBase::ForeachChild(const std::function<void (ModelsTreeItemBase*)>& handler) const
{
    foreachChild(const_cast<ModelsTreeItemBase*>(this), handler);
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
