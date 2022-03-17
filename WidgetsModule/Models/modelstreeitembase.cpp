#include "modelstreeitembase.h"

ModelsTreeItemBase::ModelsTreeItemBase(ModelsTreeItemBase* parent)
    : m_parent(parent)
{
}

ModelsTreeItemBase::~ModelsTreeItemBase()
{
    for(const auto& child : m_childs) {
        if(child->m_parent == this) {
            child->m_parent = nullptr;
        }
    }
}

ModelsTreeItemBase* ModelsTreeItemBase::FindIf(const FilterFunc& filter) const
{
    ModelsTreeItemBase* result = nullptr;
    bool found = false;
    findChildRecursive(const_cast<ModelsTreeItemBase*>(this), [filter, &result](ModelsTreeItemBase* item) -> bool {
        if(filter(item)) {
            result = item;
            return true;
        }
        return false;
    }, found);
    return result;
}

bool ModelsTreeItemBase::HasParentOrEqual(ModelsTreeItemBase* parent) const
{
    auto tmp = this;
    while (tmp->GetParent() != nullptr) {
        if (tmp == parent){
            return true;
        }
        tmp = tmp->GetParent();
    }

    return false;
}

const SharedPointer<ModelsTreeItemBase>& ModelsTreeItemBase::AddChild(const SharedPointer<ModelsTreeItemBase>& item)
{
    item->m_parent = this;
    m_childs.append(item);
    return item;
}

void ModelsTreeItemBase::RemoveChild(qint32 i)
{
    m_childs.remove(i);
}

void ModelsTreeItemBase::RemoveChilds()
{
    m_childs.clear();
}

const SharedPointer<ModelsTreeItemBase>& ModelsTreeItemBase::GetChildPtr(ModelsTreeItemBase* child) const
{
    const auto& childs = m_childs;
    auto foundIt = std::find_if(childs.begin(), childs.end(), [child](const SharedPointer<ModelsTreeItemBase>& containerItem){
        return containerItem.get() == child;
    });
    Q_ASSERT(foundIt != childs.end());
    return *foundIt;
}

void ModelsTreeItemBase::foreachChild(ModelsTreeItemBase* item, const HandlerFunc& handler, const FilterFunc& filterFunc)
{
    for(const auto& child : item->GetChilds()) {
        if (filterFunc(child.get())){
            handler(child.get());
            foreachChild(child.get(), handler, filterFunc);
        }
    }
}

void ModelsTreeItemBase::findChildRecursive(ModelsTreeItemBase* item, const FilterFunc& filterFunc, bool& found)
{
    for(const auto& child : item->GetChilds()) {
        if (filterFunc(child.get())){
            found = true;
            return;
        }
        findChildRecursive(child.get(), filterFunc, found);
        if(found) {
            return;
        }
    }
}


void ModelsTreeItemBase::ForeachParent(const HandlerFunc& handler, const FilterFunc& filterFunc) const
{
    if(m_parent == nullptr){
        return;
    }
    
    if(filterFunc(m_parent)){
        handler(m_parent);
        m_parent->ForeachParent(handler, filterFunc);
    }
}

void ModelsTreeItemBase::ForeachChild(const HandlerFunc& handler, const FilterFunc& filterFunc) const
{
    foreachChild(const_cast<ModelsTreeItemBase*>(this), handler, filterFunc);
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

Qt::CheckState ModelsTreeItemBase::IsChecked(size_t key) const {
    auto variant = GetUserData(key, "Checked");
    return variant.value<Qt::CheckState>();
}

void ModelsTreeItemBase::SetChecked(size_t key, Qt::CheckState value) {
    SetUserData(key, "Checked", value);
}

void ModelsTreeItemBase::SetUserData(size_t key, const Name& propertyName, const QVariant& value)
{
    m_userData[key][propertyName] = value;
}

QVariant ModelsTreeItemBase::GetUserData(size_t key, const Name& propertyName) const
{
    auto foundIt = m_userData.find(key);
    if(foundIt == m_userData.end()) {
        return QVariant();
    }
    auto foundPropertyIt = foundIt->find(propertyName);
    if(foundPropertyIt == foundIt->end()) {
        return QVariant();
    }
    return foundPropertyIt.value();
}

bool ModelsTreeItemBase::IsExpanded(size_t key) const
{
    return GetUserData(key, "Expanded").toBool();
}

void ModelsTreeItemBase::SetExpanded(size_t key, bool flag)
{
    SetUserData(key, "Expanded", flag);
}

void ModelsTreeItemBase::clone(ModelsTreeItemBase* toItem) const
{
    toItem->m_childs = m_childs;
    toItem->m_parent = m_parent;
    toItem->m_userData = m_userData;
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
