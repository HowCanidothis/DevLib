#ifndef VIEWMODELTREEBASE_H
#define VIEWMODELTREEBASE_H

#include <QAbstractItemModel>

#include <PropertiesModule/internal.hpp>
#include "WidgetsModule/Utils/iconsmanager.h"

class ViewModelsTreeBase : public QAbstractItemModel
{
    Q_OBJECT
    using Super = QAbstractItemModel;

public:
    ViewModelsTreeBase(QObject* parent);
    ~ViewModelsTreeBase();

    void SetData(const ModelsTreeWrapperPtr& wrapper);
    const ModelsTreeWrapperPtr& GetData() const { return m_data; }

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void ForeachModelIndexRecursive(const QModelIndex& parent, const std::function<void (const QModelIndex& child)>& handler) const;
    void ForeachModelIndex(const QModelIndex& parent, const std::function<void (const QModelIndex& child)>& handler) const;

    QSet<ModelsTreeItemBase*> AsItems(const QModelIndexList& indices) const;
    class ModelsTreeItemBase* AsItem(const QModelIndex& index) const;
    QModelIndex AsIndex(class ModelsTreeItemBase* item) const;

protected:
    ModelsTreeWrapperPtr m_data;
    IconsSvgIcon m_errorIcon;
    IconsSvgIcon m_warningIcon;
    IconsSvgIcon m_infoIcon;
};

template<class T>
class TViewModelTreeBase : public ViewModelsTreeBase
{
    using Super = ViewModelsTreeBase;
public:
    using Super::Super;

    const SharedPointer<T>& GetData() const { return m_data.Cast<T>(); }
};

#endif // VIEWMODELSTREEBASE_H
