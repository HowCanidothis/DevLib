#include "widgetstreeviewbase.h"

#include <QApplication>
#include <QHeaderView>

struct QTreeViewItem
{
    QTreeViewItem() : parentItem(-1), expanded(false), spanning(false), hasChildren(false),
                      hasMoreSiblings(false), total(0), level(0), height(0) {}
    QModelIndex index; // we remove items whenever the indexes are invalidated
    int parentItem; // parent item index in viewItems
    uint expanded : 1;
    uint spanning : 1;
    uint hasChildren : 1; // if the item has visible children (even if collapsed)
    uint hasMoreSiblings : 1;
    uint total : 28; // total number of children visible
    uint level : 16; // indentation
    int height : 16; // row height
};

using QEditorIndexHash = QHash<QWidget*, QPersistentModelIndex>;

#define DECL_WRAP(method, T, addressOffset) \
    T& method() const { return *((T*)((size_t)this + addressOffset)); }
#define DECL_WRAP_2(method, T1,T2, addressOffset) \
    T1,T2& method() const { return *((T1,T2*)((size_t)this + addressOffset)); }

class WidgetsTreeViewBasePrivate
{
public:
    DECL_WRAP(scrollDelayOffset, QPoint, 0x380)
    DECL_WRAP(hover, QPersistentModelIndex, 0x310)
    DECL_WRAP(spanning, bool, 0x464)
    DECL_WRAP(alternatingColors, bool, 0x36a)
    DECL_WRAP_2(leftAndRight, QPair<int,int>, 0x458)
    DECL_WRAP(editorIndexHash,QEditorIndexHash, 0x2b0)
    DECL_WRAP(treePosition, int, 0x4a8)
    DECL_WRAP(hoverBranch, int, 0x4a0)
    DECL_WRAP(current, int, 0x460)
    DECL_WRAP(indent, int, 0x438)
    DECL_WRAP(viewItems, QVector<QTreeViewItem>, 0x440)
    DECL_WRAP(rootDecoration, bool, 0x451)
};

static inline bool ancestorOf(QObject *widget, QObject *other)
{
    for (QObject *parent = other; parent != nullptr; parent = parent->parent()) {
        if (parent == widget)
            return true;
    }
    return false;
}

void WidgetsTreeViewBase::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const WidgetsTreeViewBase);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    QStyleOptionViewItem opt = option;
    const QPoint offset = d->scrollDelayOffset();
    const int y = option.rect.y() + offset.y();
    const QModelIndex parent = index.parent();
    const QHeaderView *header = this->header();
    const QModelIndex current = currentIndex();
    const QModelIndex hover = d->hover();
    const bool reverse = isRightToLeft();
    const QStyle::State state = opt.state;
    const bool spanning = d->spanning();
    const int left = (spanning ? header->visualIndex(0) : d->leftAndRight().first);
    const int right = (spanning ? header->visualIndex(0) : d->leftAndRight().second);
    const bool alternate = d->alternatingColors();
    const bool enabled = (state & QStyle::State_Enabled) != 0;
    const bool allColumnsShowFocus = this->allColumnsShowFocus();


    // when the row contains an index widget which has focus,
    // we want to paint the entire row as active
    bool indexWidgetHasFocus = false;
    if ((current.row() == index.row()) && !d->editorIndexHash().isEmpty()) {
        const int r = index.row();
        QWidget *fw = QApplication::focusWidget();
        for (int c = 0; c < header->count(); ++c) {
            QModelIndex idx = model()->index(r, c, parent);
            if (QWidget *editor = indexWidget(idx)) {
                if (ancestorOf(editor, fw)) {
                    indexWidgetHasFocus = true;
                    break;
                }
            }
        }
    }

    const bool widgetHasFocus = hasFocus();
    bool currentRowHasFocus = false;
    if (allColumnsShowFocus && widgetHasFocus && current.isValid()) {
        // check if the focus index is before or after the visible columns
        const int r = index.row();
        for (int c = 0; c < left && !currentRowHasFocus; ++c) {
            QModelIndex idx = model()->index(r, c, parent);
            currentRowHasFocus = (idx == current);
        }
        QModelIndex parent = model()->parent(index);
        for (int c = right; c < header->count() && !currentRowHasFocus; ++c) {
            currentRowHasFocus = (model()->index(r, c, parent) == current);
        }
    }

    // ### special case: treeviews with multiple columns draw
    // the selections differently than with only one column
    opt.showDecorationSelected = (selectionBehavior() & SelectRows)
                                 || option.showDecorationSelected;

    int width, height = option.rect.height();
    int position;
    QModelIndex modelIndex;
    const bool hoverRow = selectionBehavior() == QAbstractItemView::SelectRows
                  && index.parent() == hover.parent()
                  && index.row() == hover.row();

    QVector<int> logicalIndices;
    QVector<QStyleOptionViewItem::ViewItemPosition> viewItemPosList; // vector of left/middle/end for each logicalIndex
    calcLogicalIndices(&logicalIndices, &viewItemPosList, left, right);

    for (int currentLogicalSection = 0; currentLogicalSection < logicalIndices.count(); ++currentLogicalSection) {
        int headerSection = logicalIndices.at(currentLogicalSection);
        position = columnViewportPosition(headerSection) + offset.x();
        width = header->sectionSize(headerSection);

        if (spanning) {
            int lastSection = header->logicalIndex(header->count() - 1);
            if (!reverse) {
                width = columnViewportPosition(lastSection) + header->sectionSize(lastSection) - position;
            } else {
                width += position - columnViewportPosition(lastSection);
                position = columnViewportPosition(lastSection);
            }
        }

        modelIndex = model()->index(index.row(), headerSection, parent);
        if (!modelIndex.isValid())
            continue;
        opt.state = state;

        opt.viewItemPosition = viewItemPosList.at(currentLogicalSection);

        // fake activeness when row editor has focus
        if (indexWidgetHasFocus)
            opt.state |= QStyle::State_Active;

//        if (selectionModel()->isSelected(modelIndex))
//            opt.state |= QStyle::State_Selected;
        if (widgetHasFocus && (current == modelIndex)) {
            if (allColumnsShowFocus)
                currentRowHasFocus = true;
            else
                opt.state |= QStyle::State_HasFocus;
        }
        opt.state.setFlag(QStyle::State_MouseOver,
                          (hoverRow || modelIndex == hover)
                          && (option.showDecorationSelected || d->hoverBranch() == -1));

        if (enabled) {
            QPalette::ColorGroup cg;
            if ((model()->flags(modelIndex) & Qt::ItemIsEnabled) == 0) {
                opt.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            } else if (opt.state & QStyle::State_Active) {
                cg = QPalette::Active;
            } else {
                cg = QPalette::Inactive;
            }
            opt.palette.setCurrentColorGroup(cg);
        }

        if (alternate) {
            opt.features.setFlag(QStyleOptionViewItem::Alternate, d->current() & 1);
        }

        /* Prior to Qt 4.3, the background of the branch (in selected state and
           alternate row color was provided by the view. For backward compatibility,
           this is now delegated to the style using PE_PanelViewItemRow which
           does the appropriate fill */
        if (isTreePosition(headerSection)) {
            const int i = indentationForItem(d->current());
            QRect branches(reverse ? position + width - i : position, y, i, height);
            const bool setClipRect = branches.width() > width;
            if (setClipRect) {
                painter->save();
                painter->setClipRect(QRect(position, y, width, height));
            }
            // draw background for the branch (selection + alternate row)
            opt.rect = branches;
            if (style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, &opt, this))
                style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);

            // draw background of the item (only alternate row). rest of the background
            // is provided by the delegate
            QStyle::State oldState = opt.state;
            opt.state &= ~QStyle::State_Selected;
            opt.rect.setRect(reverse ? position : i + position, y, width - i, height);
            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);
            opt.state = oldState;

            if (d->indent() != 0)
                drawBranches(painter, branches, index);
            if (setClipRect)
                painter->restore();
        } else {
            QStyle::State oldState = opt.state;
            opt.state &= ~QStyle::State_Selected;
            opt.rect.setRect(position, y, width, height);
            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);
            opt.state = oldState;
        }

        itemDelegate(modelIndex)->paint(painter, opt, modelIndex);
        painter->setPen(m_borderColor);
        painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
    }

    if (currentRowHasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        o.state |= QStyle::State_KeyboardFocusChange;
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = option.palette.color(cg, selectionModel()->isSelected(index)
                                                 ? QPalette::Highlight : QPalette::Window);
        int x = 0;
        if (!option.showDecorationSelected)
            x = header->sectionPosition(0) + indentationForItem(d->current());
        QRect focusRect(x - header->offset(), y, header->length() - x, height);
        o.rect = style()->visualRect(layoutDirection(), viewport()->rect(), focusRect);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
        // if we show focus on all columns and the first section is moved,
        // we have to split the focus rect into two rects
        if (allColumnsShowFocus && !option.showDecorationSelected
            && header->sectionsMoved() && (header->visualIndex(0) != 0)) {
            QRect sectionRect(0, y, header->sectionPosition(0), height);
            o.rect = style()->visualRect(layoutDirection(), viewport()->rect(), sectionRect);
            style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
        }
    }
}

bool WidgetsTreeViewBase::isTreePosition(int logicalIndex) const
{
    return logicalIndex == logicalIndexForTree();
}

int WidgetsTreeViewBase::indentationForItem(int item) const
{
    Q_D(const WidgetsTreeViewBase);
    if (item < 0 || item >= d->viewItems().count())
        return 0;
    int level = d->viewItems().at(item).level;
    if (d->rootDecoration())
        ++level;
    return level * d->indent();
}

void WidgetsTreeViewBase::calcLogicalIndices(QVector<int> *logicalIndices, QVector<QStyleOptionViewItem::ViewItemPosition> *itemPositions, int left, int right) const
{
    Q_D(const WidgetsTreeViewBase);
    const int columnCount = header()->count();
    /* 'left' and 'right' are the left-most and right-most visible visual indices.
       Compute the first visible logical indices before and after the left and right.
       We will use these values to determine the QStyleOptionViewItem::viewItemPosition. */
    int logicalIndexBeforeLeft = -1, logicalIndexAfterRight = -1;
    for (int visualIndex = left - 1; visualIndex >= 0; --visualIndex) {
        int logicalIndex = header()->logicalIndex(visualIndex);
        if (!header()->isSectionHidden(logicalIndex)) {
            logicalIndexBeforeLeft = logicalIndex;
            break;
        }
    }

    for (int visualIndex = left; visualIndex < columnCount; ++visualIndex) {
        int logicalIndex = header()->logicalIndex(visualIndex);
        if (!header()->isSectionHidden(logicalIndex)) {
            if (visualIndex > right) {
                logicalIndexAfterRight = logicalIndex;
                break;
            }
            logicalIndices->append(logicalIndex);
        }
    }

    itemPositions->resize(logicalIndices->count());
    for (int currentLogicalSection = 0; currentLogicalSection < logicalIndices->count(); ++currentLogicalSection) {
        const int headerSection = logicalIndices->at(currentLogicalSection);
        // determine the viewItemPosition depending on the position of column 0
        int nextLogicalSection = currentLogicalSection + 1 >= logicalIndices->count()
                                 ? logicalIndexAfterRight
                                 : logicalIndices->at(currentLogicalSection + 1);
        int prevLogicalSection = currentLogicalSection - 1 < 0
                                 ? logicalIndexBeforeLeft
                                 : logicalIndices->at(currentLogicalSection - 1);
        QStyleOptionViewItem::ViewItemPosition pos;
        if (columnCount == 1 || (nextLogicalSection == 0 && prevLogicalSection == -1)
            || (headerSection == 0 && nextLogicalSection == -1) || d->spanning())
            pos = QStyleOptionViewItem::OnlyOne;
        else if (isTreePosition(headerSection) || (nextLogicalSection != 0 && prevLogicalSection == -1))
            pos = QStyleOptionViewItem::Beginning;
        else if (nextLogicalSection == 0 || nextLogicalSection == -1)
            pos = QStyleOptionViewItem::End;
        else
            pos = QStyleOptionViewItem::Middle;
        (*itemPositions)[currentLogicalSection] = pos;
    }
}

int WidgetsTreeViewBase::logicalIndexForTree() const
{
    Q_D(const WidgetsTreeViewBase);
    int index = d->treePosition();
    if (index < 0)
        index = header()->logicalIndex(0);
    return index;
}
