#ifndef MODELSTREEITEMBASE_H
#define MODELSTREEITEMBASE_H

#include <QIcon>

#include <SharedModule/internal.hpp>

template<class T>
class ModelsTreeBuilder
{
public:
    ModelsTreeBuilder(const SharedPointer<T>& root)
        : m_root(root)
    {
        m_path.append(root.get());
    }

    template<typename ... Args>
    ModelsTreeBuilder(Args... args)
        : m_root(::make_shared<T>(args...))
    {
        m_path.append(m_root.get());
    }

    template<typename ... Args>
    ModelsTreeBuilder& AddChild(Args... args);
    template<typename ... Args>
    ModelsTreeBuilder& Cd(Args... args);
    ModelsTreeBuilder& Up() { m_path.pop_back(); Q_ASSERT(!m_path.isEmpty()); return *this; }
    ModelsTreeBuilder& UpToRoot() { m_path.resize(1); return *this; }

    T* GetCurrent() const { return m_path.constLast(); }
    const SharedPointer<T>& GetRoot() const { return m_root; }

private:
    friend class ModelsTreeItemBase;

    SharedPointer<T> m_root;
    QVector<T*> m_path;
};

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
    const SharedPointer<ModelsTreeItemBase>& AddChild(const SharedPointer<ModelsTreeItemBase>& item);
    void InsertChilds(qint32 before, qint32 count, const std::function<SharedPointer<ModelsTreeItemBase> ()>& itemCreator);
    const SharedPointer<ModelsTreeItemBase>& InsertChild(qint32 before, const SharedPointer<ModelsTreeItemBase>& item);
    void RemoveChilds();
    void RemoveChild(qint32 i);
    void ForeachChild(const HandlerFunc& handler, const FilterFunc& filterFunc = [](ModelsTreeItemBase*){return true;}) const;
    void ForeachParent(const HandlerFunc& handler, const FilterFunc& filterFunc = [](ModelsTreeItemBase*){return true;}) const;
    const SharedPointer<ModelsTreeItemBase>& GetChildPtr(ModelsTreeItemBase* child) const;

    virtual QString GetLabel() const { return QString::number(GetRow()); }
    virtual QIcon GetIcon() const { return QIcon(); }
    virtual QFont GetFont() const { return QFont(); }
    virtual qint32 GetType() const { return -1; }

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

    static const Name ExpandedKey;
    static const Name CheckedKey;

private:
    static void foreachChild(ModelsTreeItemBase* item, const HandlerFunc& handler, const FilterFunc& filterFunc = nullptr);
    static void findChildRecursive(ModelsTreeItemBase* item, const FilterFunc& filterFunc, bool& found);

protected:
    virtual void clone(ModelsTreeItemBase* item) const;
};

template<class T> template<typename ... Args>
ModelsTreeBuilder<T>& ModelsTreeBuilder<T>::AddChild(Args... args)
{
    const auto& child = GetCurrent()->AddChild(::make_shared<T>(args...));
    return *this;
}

template<class T> template<typename ... Args>
ModelsTreeBuilder<T>& ModelsTreeBuilder<T>::Cd(Args... args)
{
    const auto& child = GetCurrent()->AddChild(::make_shared<T>(args...));
    m_path.append((T*)child.get());
    return *this;
}

class ModelsStandardTreeItem : public ModelsTreeItemBase // TODO. Need to finish this
{
public:
    ModelsStandardTreeItem(const FTranslationHandler& label = nullptr);

    QString GetLabel() const override;

private:
    FTranslationHandler m_label;
};

Q_DECLARE_METATYPE(SharedPointer<ModelsTreeItemBase>)

#endif // MODELSTREEITEM_H
