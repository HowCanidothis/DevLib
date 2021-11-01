#include "widgetsresizabletableview.h"

#include <QLayout>
#include <QHeaderView>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsResizableTableView::WidgetsResizableTableView(QWidget* parent)
    : Super(parent)
{
    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::Show || event->type() == QEvent::StyleChange) {
            invalidateLayout();
        }
        return false;
    });
}

void WidgetsResizableTableView::setModel(QAbstractItemModel* sourceModel)
{
    Super::setModel(sourceModel);

    m_connections.Clear();
    m_connections.connect(model(), &QAbstractItemModel::modelReset, [this]{ invalidateLayout(); });
    m_connections.connect(model(), &QAbstractItemModel::rowsInserted, [this]{ invalidateLayout(); });
    m_connections.connect(model(), &QAbstractItemModel::rowsRemoved, [this]{ invalidateLayout(); });
    invalidateLayout();
}

void WidgetsResizableTableView::invalidateLayout()
{
    auto sh = sizeHint();
    setFixedHeight(sh.height());
}

QSize WidgetsResizableTableView::sizeHint() const
{
    int w = verticalHeader()->width() + contentsMargins().left() + contentsMargins().right();
    for (int c = 0; c < model()->columnCount(); ++c) w += columnWidth(c);

    int h = horizontalHeader()->height() + contentsMargins().top() + contentsMargins().bottom();
    for (int r = 0; r < model()->rowCount(); r++) h += rowHeight(r);
    return {w, h};
}

QSize WidgetsResizableTableView::minimumSizeHint() const
{
    return sizeHint();
}
