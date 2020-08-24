#ifndef MODELSTREEITEMBASE_H
#define MODELSTREEITEMBASE_H

#include <QIcon>

#include <SharedModule/internal.hpp>

class ModelsTreeItemBase
{
    using HandlerFunc = std::function<void (ModelsTreeItemBase*)>;
    using SkipFunc = std::function<bool(ModelsTreeItemBase*)>;

    friend class ModelsTree;
    template<class T> friend struct Serializer;

    ModelsTreeItemBase* m_parent;
    mutable QHash<qint64,Qt::CheckState> m_checkedMap;
    QVector<SharedPointer<ModelsTreeItemBase>> m_childs;

public:
    ModelsTreeItemBase(ModelsTreeItemBase* parent = nullptr);
    ModelsTreeItemBase(const ModelsTreeItemBase& o);
    ModelsTreeItemBase& operator= (ModelsTreeItemBase& o);

    const QVector<SharedPointer<ModelsTreeItemBase>>& GetChilds() const { return m_childs; }
    ModelsTreeItemBase* GetParent() const { return m_parent; }
    qint32 GetRow() const;
    qint32 GetParentRow() const;

    Qt::CheckState GetChecked(const qint64& key) const;
    void SetChecked(const qint64& key, Qt::CheckState value);

    void AddChild(const SharedPointer<ModelsTreeItemBase>& item);
    void RemoveChilds();
    void RemoveChild(qint32 i);
    void ForeachChild(const HandlerFunc& handler, const SkipFunc* skipFunc = nullptr) const;
    void ForeachChildAfter(const HandlerFunc& handler, const SkipFunc* skipFunc = nullptr) const;

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
    static void foreachChild(ModelsTreeItemBase* item, const HandlerFunc& handler, const SkipFunc* skipFunc = nullptr);
    static void foreachChildAfter(ModelsTreeItemBase* item, const HandlerFunc& handler, const SkipFunc* skipFunc = nullptr);

protected:
    virtual void clone(ModelsTreeItemBase* item) const;
};

using ModelsTreeItemBasePtr = SharedPointer<ModelsTreeItemBase>;

#endif // MODELSTREEITEM_H
