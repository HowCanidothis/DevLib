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

public:
    ModelsTreeItemBase(ModelsTreeItemBase* parent = nullptr);

    const QVector<SharedPointer<ModelsTreeItemBase>>& GetChilds() const { return m_childs; }
    ModelsTreeItemBase* GetParent() const { return m_parent; }
    qint32 GetRow() const;
    qint32 GetParentRow() const;

    void AddChild(const SharedPointer<ModelsTreeItemBase>& item);
    void RemoveChilds();
    void RemoveChild(qint32 i);
    void ForeachChild(const std::function<void (ModelsTreeItemBase*)>& handler) const;

    virtual QString GetLabel() const { return QString::number(GetRow()); }
    virtual QIcon GetIcon() const { return QIcon(); }

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
    static void foreachChild(ModelsTreeItemBase* item, const std::function<void (ModelsTreeItemBase*)>& handler);

protected:
    virtual void clone(ModelsTreeItemBase* item) const;
};

using ModelsTreeItemBasePtr = SharedPointer<ModelsTreeItemBase>;

#endif // MODELSTREEITEM_H
