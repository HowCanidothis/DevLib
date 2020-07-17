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

    void AddChild(ModelsTreeBaseItem* item);

    template<class T> T* As() { return reinterpret_cast<T*>(this); }
    template<class T> const T* As() const { return reinterpret_cast<const T*>(this); }

    virtual QString GetLabel() const = 0;
    virtual QIcon GetIcon() const { return QIcon(); }

    ModelsTreeBaseItem* Parent;
    ArrayPointers<ModelsTreeBaseItem> Childs;
};

class ModelsTreeBaseDefaultItem : public ModelsTreeBaseItem
{
    using Super = ModelsTreeBaseItem;
public:
    QString Label;

    template<class T>
    T* AddChild()
    {
        auto* result = new T();
        result->Parent = this;
        Childs.Append(result);
        return result;
    }
    void AddChild(ModelsTreeBaseItem* item) { Super::AddChild(item); }
    ModelsTreeBaseDefaultItem* AddChild() { return AddChild<ModelsTreeBaseDefaultItem>(); }

    QString GetLabel() const override { return Label; }
};

class ModelsTreeBase : public QAbstractItemModel
{
public:
    ModelsTreeBase();

    void AddChild(const QModelIndex& parent, ModelsTreeBaseItem* item);
    void Update(const std::function<ModelsTreeBaseItem* (ModelsTreeBaseItem*)>& resetFunction);
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    ModelsTreeBaseItem* AsItem(const QModelIndex& index) const;

protected:
    ScopedPointer<ModelsTreeBaseItem> m_root;
};

#endif // MODELSTREEBASE_H
