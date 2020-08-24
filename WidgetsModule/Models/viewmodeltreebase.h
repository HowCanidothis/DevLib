#ifndef VIEWMODELTREEBASE_H
#define VIEWMODELTREEBASE_H

#include <QAbstractItemModel>

#include "wrappers.h"


class ViewModelTreeBase : public QAbstractItemModel
{
public:
    ViewModelTreeBase();
    ~ViewModelTreeBase();

    void SetData(const ModelsTreeWrapperPtr& wrapper);
    const ModelsTreeWrapperPtr& GetData() const { return m_data; }

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    class ModelsTreeItemBase* AsItem(const QModelIndex& index) const;
    QModelIndex AsIndex(class ModelsTreeItemBase* item) const;

protected:
    ModelsTreeWrapperPtr m_data;
};

template<class T>
class TViewModelTreeBase : public ViewModelTreeBase
{
public:
    const SharedPointer<T>& GetData() const { return m_data.Cast<T>(); }
};

#endif // VIEWMODELSTREEBASE_H
