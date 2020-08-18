#ifndef MODELSTREEITEMBASE_H
#define MODELSTREEITEMBASE_H

#include <QIcon>

#include <SharedModule/internal.hpp>

class ModelsTreeItemBase
{
    friend class ModelsTree;
    template<class T> friend struct Serializer;

    static int m_idCounter;

    int m_id;
    Qt::CheckState m_checked;
    ModelsTreeItemBase* m_parent;
    QVector<SharedPointer<ModelsTreeItemBase>> m_childs;

public:
    enum CheckMode {
        CurrentNode,
        CurrentWithChilds
    };

    ModelsTreeItemBase(ModelsTreeItemBase* parent = nullptr);
    ModelsTreeItemBase(const ModelsTreeItemBase& o) {
        m_id = o.m_id;
        m_parent = o.m_parent;
        m_childs = o.m_childs;
    }
    ModelsTreeItemBase& operator= (ModelsTreeItemBase& o) {
        m_id = o.m_id;
        m_parent = o.m_parent;
        m_childs = o.m_childs;
        return *this;
    }

    const QVector<SharedPointer<ModelsTreeItemBase>>& GetChilds() const { return m_childs; }
    ModelsTreeItemBase* GetParent() const { return m_parent; }
    qint32 GetRow() const;
    qint32 GetParentRow() const;
    int Id() const { return m_id; }
    Qt::CheckState Checked() const { return m_checked; }
    void SetChecked(Qt::CheckState value, CheckMode mode, std::function<void(ModelsTreeItemBase*)> func) {
        if (value == Qt::Checked){//надо ли?
            m_checked = value;
            func(this);
            onCheckdChanged(value, mode, func);
        } else {
            onCheckdChanged(value, mode, func);
            m_checked = value;
            func(this);
        }
    }

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
    virtual void onCheckdChanged(Qt::CheckState value, CheckMode mode, std::function<void(ModelsTreeItemBase*)> func) {
        if (mode == CurrentWithChilds){
            for (auto ch : m_childs) {
                ch->SetChecked(value, mode, func);
            }
        }
    }
    virtual void clone(ModelsTreeItemBase* item) const;
};

using ModelsTreeItemBasePtr = SharedPointer<ModelsTreeItemBase>;

#endif // MODELSTREEITEM_H
