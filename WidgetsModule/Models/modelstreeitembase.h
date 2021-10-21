#ifndef MODELSTREEITEMBASE_H
#define MODELSTREEITEMBASE_H

#include <QIcon>

#include <SharedModule/internal.hpp>

class ModelsTreeItemBase
{
    friend class ModelsTree;
    template<class T> friend struct Serializer;

    ModelsTreeItemBase* m_parent;
    QVector<SharedPointer<ModelsTreeItemBase>> m_childs;
    QHash<size_t, QHash<Name, QVariant>> m_userData;

public:
    using HandlerFunc = std::function<void (ModelsTreeItemBase*)>;
    using FilterFunc = std::function<bool(ModelsTreeItemBase*)>;

    ModelsTreeItemBase(ModelsTreeItemBase* parent = nullptr);
    virtual ~ModelsTreeItemBase();

    const QVector<SharedPointer<ModelsTreeItemBase>>& GetChilds() const { return m_childs; }
    ModelsTreeItemBase* GetParent() const { return m_parent; }
    bool HasParentOrEqual(ModelsTreeItemBase* parent) const;
    qint32 GetRow() const;
    qint32 GetParentRow() const;

    Qt::CheckState IsChecked(size_t key) const;
    void SetChecked(size_t key, Qt::CheckState value);
    void ResetUserData(size_t key) { m_userData.remove(key); }
    void SetUserData(size_t key, const Name& propertyName, const QVariant& value);
    QVariant GetUserData(size_t key, const Name& propertyName) const;

    bool IsExpanded(size_t key) const;
    void SetExpanded(size_t key, bool flag);

    ModelsTreeItemBase* FindIf(const FilterFunc& filter) const;
    void AddChild(const SharedPointer<ModelsTreeItemBase>& item);
    void RemoveChilds();
    void RemoveChild(qint32 i);
    void ForeachChild(const HandlerFunc& handler, const FilterFunc& filterFunc = [](ModelsTreeItemBase*){return true;}) const;
    void ForeachParent(const HandlerFunc& handler, const FilterFunc& filterFunc = [](ModelsTreeItemBase*){return true;}) const;
    const SharedPointer<ModelsTreeItemBase>& GetChildPtr(ModelsTreeItemBase* child) const;

    virtual QString GetLabel() const { return QString::number(GetRow()); }
    virtual QIcon GetIcon() const { return QIcon(); }
    virtual QFont GetFont() const { return QFont(); }

    const SharedPointer<ModelsTreeItemBase>& AsPtr() { Q_ASSERT(GetParent() != nullptr); return GetParent()->GetChildPtr(this); }
    template<class T>
    T* As() { return reinterpret_cast<T*>(this); }
    template<class T>
    const T* As() const { return reinterpret_cast<const T*>(this); }

    template<class T>
    SharedPointer<T> Clone() const
    {
        auto cloned = ::make_shared<T>();
        clone(cloned.get());
        return cloned;
    }

    QVector<ModelsTreeItemBase*> GetPath() const;

private:
    static void foreachChild(ModelsTreeItemBase* item, const HandlerFunc& handler, const FilterFunc& filterFunc = nullptr);
    static void findChildRecursive(ModelsTreeItemBase* item, const FilterFunc& filterFunc, bool& found);

protected:
    virtual void clone(ModelsTreeItemBase* item) const;
};

Q_DECLARE_METATYPE(SharedPointer<ModelsTreeItemBase>)

#endif // MODELSTREEITEM_H
