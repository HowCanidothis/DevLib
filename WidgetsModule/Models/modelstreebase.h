#ifndef MODELSTREEBASE_H
#define MODELSTREEBASE_H


#include <QAbstractItemModel>

#include <SharedModule/internal.hpp>

class ModelsTreeBaseItem
{
public:
    ModelsTreeBaseItem(ModelsTreeBaseItem* parent = nullptr)
        : Parent(parent)
    {}
    virtual ~ModelsTreeBaseItem() {}

    qint32 GetRow() const;
    qint32 GetParentRow() const;

    void AddChild(const SharedPointer<ModelsTreeBaseItem>& item);

    template<class T> T* As() { return reinterpret_cast<T*>(this); }
    template<class T> const T* As() const { return reinterpret_cast<const T*>(this); }

    virtual QString GetLabel() const = 0;
    virtual QIcon GetIcon() const { return QIcon(); }

    ModelsTreeBaseItem* Parent;
    QVector<SharedPointer<ModelsTreeBaseItem>> Childs;
};

class ModelsTreeBaseDefaultItem : public ModelsTreeBaseItem
{
    using Super = ModelsTreeBaseItem;
public:
    QString Label;

    template<class T>
    T* AddChild()
    {
        auto result = ::make_shared<T>();
        result->Parent = this;
        Childs.append(result);
        return result.get();
    }
    void AddChild(const SharedPointer<ModelsTreeBaseItem>& item) { Super::AddChild(item); }
    ModelsTreeBaseDefaultItem* AddChild() { return AddChild<ModelsTreeBaseDefaultItem>(); }

    QString GetLabel() const override { return Label; }
};

using ModelsTreeBaseItemPtr = SharedPointer<ModelsTreeBaseItem>;

class ModelsTreeBase : public QAbstractItemModel
{
public:
    ModelsTreeBase();

    void AddChild(const QModelIndex& parent, const SharedPointer<ModelsTreeBaseItem>& item);
    void Update(const std::function<ModelsTreeBaseItemPtr (const ModelsTreeBaseItemPtr&)>& resetFunction);
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    ModelsTreeBaseItem* AsItem(const QModelIndex& index) const;
    ModelsTreeBaseItemPtr AsItemPtr(const QModelIndex& index) const;
    const SharedPointer<ModelsTreeBaseItem>& GetRoot() const { return m_root; }
    SharedPointer<ModelsTreeBaseItem>& GetRoot() { return m_root; }

protected:
    SharedPointer<ModelsTreeBaseItem> m_root;
};

#endif // MODELSTREEBASE_H
