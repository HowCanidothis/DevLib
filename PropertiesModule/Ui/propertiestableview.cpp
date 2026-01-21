#include "propertiestableview.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>

#include "propertiestablemodel.h"
#include "propertiesdelegate.h"

PropertiesTableModel* PropertiesTableView::GetModel() const
{
    return m_propertiesModel;
}

PropertiesTableView::PropertiesTableView(QWidget* parent, Qt::WindowFlags flags)
    : Super(parent)
{
    setWindowFlags(windowFlags() | flags);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    horizontalHeader()->setStretchLastSection(true);

    m_propertiesModel = new PropertiesTableModel(this);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(m_propertiesModel);
    setModel(proxy);

//    setItemDelegate(new PropertiesDelegate(this));
}
