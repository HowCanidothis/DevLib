#ifndef WIDGETSTREEVIEWBASE_H
#define WIDGETSTREEVIEWBASE_H

#include <QTreeView>

class WidgetsTreeViewBasePrivate;

class WidgetsTreeViewBase : public QTreeView
{
    Q_OBJECT
    using Super = QTreeView;

    Q_PROPERTY(QColor gridlineColor MEMBER m_borderColor)

public:
    using Super::Super;

    void drawRow(QPainter *painter,
                         const QStyleOptionViewItem &options,
                         const QModelIndex &index) const;

private:
    Q_DECLARE_PRIVATE(WidgetsTreeViewBase);
    void calcLogicalIndices(QVector<int> *logicalIndices, QVector<QStyleOptionViewItem::ViewItemPosition> *itemPositions, int left, int right) const;
    int logicalIndexForTree() const;
    bool isTreePosition(int logicalIndex) const;
    int indentationForItem(int item) const;

private:
    QColor m_borderColor;
};

#endif // WIDGETSTREEVIEWBASE_H
