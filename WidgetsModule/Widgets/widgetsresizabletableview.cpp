#include "widgetsresizabletableview.h"

#include <QLayout>
#include <QHeaderView>

WidgetsResizableTableView::WidgetsResizableTableView(QWidget* parent)
    : Super(parent)
{
}

void WidgetsResizableTableView::setModel(QAbstractItemModel* sourceModel)
{
    Super::setModel(sourceModel);

    m_connections.Clear();
    m_connections.connect(model(), &QAbstractItemModel::modelReset, [this]{ setFixedSize(sizeHint()); });
    m_connections.connect(model(), &QAbstractItemModel::rowsInserted, [this]{ setFixedSize(sizeHint()); });
    m_connections.connect(model(), &QAbstractItemModel::rowsRemoved, [this]{ setFixedSize(sizeHint()); });
    setFixedSize(sizeHint());
}

QSize WidgetsResizableTableView::sizeHint() const
{
    int w = verticalHeader()->width() + contentsMargins().left() + contentsMargins().right();
    for (int c = 0; c < model()->columnCount(); ++c) w += columnWidth(c);

    int h = horizontalHeader()->height() + contentsMargins().top() + contentsMargins().bottom();
    for (int r = 0; r < model()->rowCount(); r++) h += rowHeight(r);
    return {w, h};
}
