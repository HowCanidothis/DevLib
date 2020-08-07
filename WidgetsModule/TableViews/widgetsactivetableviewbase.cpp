#include "widgetsactivetableviewbase.h"

WidgetsActiveTableViewBase::WidgetsActiveTableViewBase(QWidget* parent)
    : Super(parent)
{}

void WidgetsActiveTableViewBase::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Super::selectionChanged(selected, deselected);
    OnSelectionChanged(selectionModel()->selectedIndexes().size());
}

void WidgetsActiveTableViewBase::mousePressEvent(QMouseEvent* event)
{
    Super::mousePressEvent(event);
    OnActivated();
}

