#include "widgetstableviewbase.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QBitArray>
#include <QStyledItemDelegate>
#include <QApplication>

#include "WidgetsModule/widgetsdeclarations.h"
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Delegates/delegates.h"

IMPLEMENT_GLOBAL_CHAR_1(OverridenEditorEvent)
using EditorEvent = std::function<void (const FAction&)>;
using WidgetsTableViewBaseOverridenEditorEvent = SharedPointer<EditorEvent>;
Q_DECLARE_METATYPE(WidgetsTableViewBaseOverridenEditorEvent)

struct WidgetsTableViewBaseSpan
{
    int m_top;
    int m_left;
    int m_bottom;
    int m_right;
    bool will_be_deleted;
    inline int top() const { return m_top; }
    inline int left() const { return m_left; }
    inline int bottom() const { return m_bottom; }
    inline int right() const { return m_right; }
    inline int height() const { return m_bottom - m_top + 1; }
    inline int width() const { return m_right - m_left + 1; }
};

struct WidgetsTableViewBaseSpanCollection
{
    typedef std::list<WidgetsTableViewBaseSpan*> SpanList;
    SpanList spans; //lists of all spans
    //the indexes are negative so the QMap::lowerBound do what i need.
    typedef QMap<int, WidgetsTableViewBaseSpan*> SubIndex;
    typedef QMap<int, SubIndex> Index;
    Index index;

    QSet<WidgetsTableViewBaseSpan*> spansInRect(int x, int y, int w, int h) const
    {
        QSet<WidgetsTableViewBaseSpan*> list;
        if(index.isEmpty()) {
            return list;
        }
        Index::const_iterator it_y = index.lowerBound(-y);
        if(it_y == index.end())
            --it_y;
        while(-it_y.key() <= y + h) {
            SubIndex::const_iterator it_x = (*it_y).lowerBound(-x);
            if (it_x == (*it_y).end())
                --it_x;
            while(-it_x.key() <= x + w) {
                WidgetsTableViewBaseSpan *s = *it_x;
                if (s->bottom() >= y && s->right() >= x)
                    list << s;
                if (it_x == (*it_y).begin())
                    break;
                --it_x;
            }
            if(it_y == index.begin())
                break;
            --it_y;
        }
        return list;
    }

    WidgetsTableViewBaseSpan* spanAt(int x, int y) const
    {
        Index::const_iterator it_y = index.lowerBound(-y);
        if (it_y == index.end())
            return nullptr;
        SubIndex::const_iterator it_x = (*it_y).lowerBound(-x);
        if (it_x == (*it_y).end())
            return nullptr;
        WidgetsTableViewBaseSpan* span = *it_x;
        if (span->right() >= x && span->bottom() >= y)
            return span;
        return nullptr;
    }
};

class WidgetsTableViewBasePrivate
{
public:
    const QPoint& scrollDelayOffset() const { return *((QPoint*)((size_t)this + 0x380)); }
//    qint32 top() const { return *((qint32*)((size_t)this + 0x244)); }
    const QPersistentModelIndex& hoverIndex() const { return *((QPersistentModelIndex*)((size_t)this + 0x310)); }
    const QRect& dropIndicatorRect() const { return *((QRect*)((size_t)this + 0x31c)); }
    const WidgetsTableViewBaseSpanCollection& spans() const { return *((WidgetsTableViewBaseSpanCollection*)((size_t)this + 0x420)); }
    bool hasSpans() { return !spans().spans.empty(); }
};

WidgetsTableViewBase::WidgetsTableViewBase(QWidget* parent)
    : Super(parent)
{
    horizontalHeader()->setFocusProxy(this);
    verticalHeader()->setFocusProxy(this);
    setMouseTracking(true);

    m_selectionColor.OnChanged.ConnectCombined(CDL, [this]{
        update();
    }, m_hoverColor);

    setItemDelegate(new QStyledItemDelegateBase(this));
}

WidgetsTableViewBase::~WidgetsTableViewBase()
{

}

void WidgetsTableViewBase::setModel(QAbstractItemModel *model)
{
    Super::setModel(model);
    m_modelConnections.connect(selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection&, const QItemSelection&) {
        update();
    });
}

void WidgetsTableViewBase::OverrideEditorEvent(const std::function<void (const FAction&)>& action)
{
    setProperty(OverridenEditorEvent, QVariant::fromValue(::make_shared<EditorEvent>(action)));
}

void WidgetsTableViewBase::commitData(QWidget* editor)
{
    m_editorIndex = currentIndex();
    Super::commitData(editor);
}

void WidgetsTableViewBase::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
    qint32 crow;
    QHeaderView* header;
    qint32 vcolumn;
    std::function<QModelIndex (qint32 row, qint32 column)> indexExtractor;
    std::function<qint32 (const QModelIndex&)> columnExtractor;
    QModelIndex index = m_editorIndex;
    qint32 rcount;
    qint32 ccount;

    FAction findNext;
    FAction findNextRow;
    QModelIndex nextToSelect;
    qint32 increment;
    if(hint == QAbstractItemDelegate::EditNextItem) {
        increment = 1;
        findNext = [&]{
            for(qint32 i(vcolumn); i < ccount; ++i) {
                auto cIndex = indexExtractor(crow, i);
                if(!header->isSectionHidden(columnExtractor(cIndex)) && (cIndex.flags() & Qt::ItemIsEditable)) {
                    nextToSelect = cIndex;
                    break;
                }
            }
        };
        findNextRow = [&]{
            vcolumn = 0;
            auto c = rcount - crow;
            for(qint32 i(0); i < c; ++i) {
                ++crow;
                findNext();
                if(nextToSelect.isValid()) {
                    break;
                }
            }
        };
    } else if(hint == QAbstractItemDelegate::EditPreviousItem) {
        increment = -1;
        findNext = [&]{
            for(qint32 i(vcolumn); i >= 0; --i) {
                auto cIndex = indexExtractor(crow, i);
                if(!header->isSectionHidden(columnExtractor(cIndex)) && (cIndex.flags() & Qt::ItemIsEditable)) {
                    nextToSelect = cIndex;
                    break;
                }
            }
        };
        findNextRow = [&]{
            vcolumn = ccount - 1;
            auto c = crow;
            for(qint32 i(0); i < c; ++i) {
                --crow;
                findNext();
                if(nextToSelect.isValid()) {
                    break;
                }
            }
        };
    }

    if(findNext == nullptr) {
        Super::closeEditor(editor, hint);
        return;
    }

    if(model()->property(WidgetProperties::InvertedModel).toBool()) {
        crow = index.column();
        header = verticalHeader();
        vcolumn = header->visualIndex(index.row()) + increment;
        indexExtractor = [this, header](qint32 crow, qint32 i) {
            return model()->index(header->logicalIndex(i), crow);
        };
        columnExtractor = [](const QModelIndex& i) {
            return i.row();
        };
        ccount = model()->rowCount();
        rcount = model()->columnCount();
    } else {
        crow = index.row();
        header = horizontalHeader();
        vcolumn = header->visualIndex(index.column()) + increment;
        indexExtractor = [this, header](qint32 crow, qint32 i) {
            return model()->index(crow, header->logicalIndex(i));
        };
        columnExtractor = [](const QModelIndex& i) {
            return i.column();
        };
        ccount = model()->columnCount();
        rcount = model()->rowCount();
    }
    Super::closeEditor(editor, QAbstractItemDelegate::NoHint);

    findNext();

    if(!nextToSelect.isValid()) {
        findNextRow();
    }

    if(!nextToSelect.isValid()) {
        return;
    }

    auto editItem = [this](const QModelIndex& newRowModelIndex){
        auto faction = [this, newRowModelIndex]{
            auto indexFlags = newRowModelIndex.flags();
            selectionModel()->setCurrentIndex(newRowModelIndex, QItemSelectionModel::SelectCurrent);

            // currentChanged signal would have already started editing
            if (indexFlags & Qt::ItemIsEditable
                && (!(editTriggers() & QAbstractItemView::CurrentChanged))) {
                edit(newRowModelIndex);
            }
        };
        auto e = property(OverridenEditorEvent);
        if(e.isValid()) {
            (*e.value<WidgetsTableViewBaseOverridenEditorEvent>())(faction);
        } else {
            faction();
        }
    };

    editItem(nextToSelect);
}

void WidgetsTableViewBase::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if(state() != EditingState) {
            edit(currentIndex());
        }
    } else if(event->modifiers() & Qt::ControlModifier && event->modifiers() & Qt::ShiftModifier) {
        auto* selModel = selectionModel();
        if(model() != nullptr && selModel != nullptr) {
            auto ci = currentIndex();
            auto selectUp = [&]{
                auto cr = ci.row();
                QItemSelection toSelect;
                for(const auto& mi : WidgetTableViewWrapper(this).SelectedColumnsSet()) {
                    toSelect.append(QItemSelectionRange(model()->index(0, mi), model()->index(cr, mi)));
                }
                selModel->select(toSelect, QItemSelectionModel::ClearAndSelect);
            };
            auto selectDown = [&]{
                auto cr = ci.row();
                QItemSelection toSelect;
                auto rc = model()->rowCount();
                if(model()->index(rc - 1, 0).data(LastEditRowRole).toBool()) {
                    --rc;
                }
                for(const auto& mi : WidgetTableViewWrapper(this).SelectedColumnsSet()) {
                    toSelect.append(QItemSelectionRange(model()->index(cr, mi), model()->index(rc, mi)));
                }
                selModel->select(toSelect, QItemSelectionModel::ClearAndSelect);
            };

            if(event->key() == Qt::Key_Up) {
                selectUp();
                return;
            } else if(event->key() == Qt::Key_Down) {
                selectDown();
                return;
            }
        }

    }
    Super::keyPressEvent(event);
}

void WidgetsTableViewBase::paintEvent(QPaintEvent* event)
{
    Q_D(WidgetsTableViewBase);

    QStyleOptionViewItem option = viewOptions();
    const QPoint offset = d->scrollDelayOffset();
    const bool showGrid = this->showGrid();
    const int gridSize = showGrid ? 1 : 0;
    const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
    const QColor gridColor = QColor::fromRgba(static_cast<QRgb>(gridHint));
    const QPen gridPen = QPen(gridColor, 0, gridStyle());
    const QHeaderView *verticalHeader = this->verticalHeader();
    const QHeaderView *horizontalHeader = this->horizontalHeader();
    const bool alternate = this->alternatingRowColors();
    const bool rightToLeft = isRightToLeft();

    QPainter painter(this->viewport());
    QPen old = painter.pen();

    // if there's nothing to do, clear the area and return
    if (horizontalHeader->count() == 0 || verticalHeader->count() == 0 || !this->itemDelegate())
        return;

    const int x = horizontalHeader->length() - horizontalHeader->offset() - (rightToLeft ? 0 : 1);
    const int y = verticalHeader->length() - verticalHeader->offset() - 1;

    //firstVisualRow is the visual index of the first visible row.  lastVisualRow is the visual index of the last visible Row.
    //same goes for ...VisualColumn
    int firstVisualRow = qMax(verticalHeader->visualIndexAt(0),0);
    int lastVisualRow = verticalHeader->visualIndexAt(verticalHeader->height());
    if (lastVisualRow == -1)
        lastVisualRow = this->model()->rowCount(this->rootIndex()) - 1;

    int firstVisualColumn = horizontalHeader->visualIndexAt(0);
    int lastVisualColumn = horizontalHeader->visualIndexAt(horizontalHeader->width());
    if (rightToLeft)
        qSwap(firstVisualColumn, lastVisualColumn);
    if (firstVisualColumn == -1)
        firstVisualColumn = 0;
    if (lastVisualColumn == -1)
        lastVisualColumn = horizontalHeader->count() - 1;

    QBitArray drawn((lastVisualRow - firstVisualRow + 1) * (lastVisualColumn - firstVisualColumn + 1));

    const QRegion region = event->region().translated(offset);

    if (d->hasSpans()) {
        drawAndClipSpans(region, &painter, option, &drawn,
                             firstVisualRow, lastVisualRow, firstVisualColumn, lastVisualColumn, gridColor);
    }

    for (QRect dirtyArea : region) {
        dirtyArea.setBottom(qMin(dirtyArea.bottom(), int(y)));
        if (rightToLeft) {
            dirtyArea.setLeft(qMax(dirtyArea.left(), viewport()->width() - int(x)));
        } else {
            dirtyArea.setRight(qMin(dirtyArea.right(), int(x)));
        }
        // dirtyArea may be invalid when the horizontal header is not stretched
        if (!dirtyArea.isValid())
            continue;

        // get the horizontal start and end visual sections
        int left = horizontalHeader->visualIndexAt(dirtyArea.left());
        int right = horizontalHeader->visualIndexAt(dirtyArea.right());
        if (rightToLeft)
            qSwap(left, right);
        if (left == -1) left = 0;
        if (right == -1) right = horizontalHeader->count() - 1;

        // get the vertical start and end visual sections and if alternate color
        int bottom = verticalHeader->visualIndexAt(dirtyArea.bottom());
        if (bottom == -1) bottom = verticalHeader->count() - 1;
        int top = 0;
        bool alternateBase = false;
        if (alternate && verticalHeader->sectionsHidden()) {
            const int verticalOffset = verticalHeader->offset();
            int row = verticalHeader->logicalIndex(top);
            for (int y = 0;
                 ((y += verticalHeader->sectionSize(top)) <= verticalOffset) && (top < bottom);
                 ++top) {
                row = verticalHeader->logicalIndex(top);
                if (alternate && !verticalHeader->isSectionHidden(row))
                    alternateBase = !alternateBase;
            }
        } else {
            top = verticalHeader->visualIndexAt(dirtyArea.top());
            alternateBase = (top & 1) && alternate;
        }
        if (top == -1 || top > bottom)
            continue;

        // Paint each row item
        for (int visualRowIndex = top; visualRowIndex <= bottom; ++visualRowIndex) {
            int row = verticalHeader->logicalIndex(visualRowIndex);
            if (verticalHeader->isSectionHidden(row))
                continue;
            int rowY = rowViewportPosition(row);
            rowY += offset.y();
            int rowh = rowHeight(row) - gridSize;

            // Paint each column item
            for (int visualColumnIndex = left; visualColumnIndex <= right; ++visualColumnIndex) {
                int currentBit = (visualRowIndex - firstVisualRow) * (lastVisualColumn - firstVisualColumn + 1)
                        + visualColumnIndex - firstVisualColumn;

                if (currentBit < 0 || currentBit >= drawn.size() || drawn.testBit(currentBit))
                    continue;
                drawn.setBit(currentBit);

                int col = horizontalHeader->logicalIndex(visualColumnIndex);
                if (horizontalHeader->isSectionHidden(col))
                    continue;
                int colp = columnViewportPosition(col);
                colp += offset.x();
                int colw = columnWidth(col) - gridSize;

                const QModelIndex index = this->model()->index(row, col, this->rootIndex());
                if (index.isValid()) {
                    option.rect = QRect(colp + (showGrid && rightToLeft ? 1 : 0), rowY, colw, rowh);
                    if (alternate) {
                        if (alternateBase)
                            option.features |= QStyleOptionViewItem::Alternate;
                        else
                            option.features &= ~QStyleOptionViewItem::Alternate;
                    }
                    this->drawCell(&painter, option, index);
                }
            }
            alternateBase = !alternateBase && alternate;
        }

        //            if (showGrid) {
        // Find the bottom right (the last rows/columns might be hidden)
        while (verticalHeader->isSectionHidden(verticalHeader->logicalIndex(bottom))) --bottom;
        painter.setPen(gridPen);
        // Paint each row
        for (int visualIndex = top; visualIndex <= bottom; ++visualIndex) {
            int row = verticalHeader->logicalIndex(visualIndex);
            if (verticalHeader->isSectionHidden(row))
                continue;
            int rowY = rowViewportPosition(row);
            rowY += offset.y();
            int rowh = rowHeight(row) - gridSize - 1;
            painter.drawLine(dirtyArea.left(), rowY + rowh, dirtyArea.right(), rowY + rowh);
        }

        //                // Paint each column
        //                for (int h = left; h <= right; ++h) {
        //                    int col = horizontalHeader->logicalIndex(h);
        //                    if (horizontalHeader->isSectionHidden(col))
        //                        continue;
        //                    int colp = columnViewportPosition(col);
        //                    colp += offset.x();
        //                    if (!rightToLeft)
        //                        colp +=  columnWidth(col) - gridSize;
        //                    painter.drawLine(colp, dirtyArea.top(), colp, dirtyArea.bottom());
        //                }
        //            }
    }

    painter.setClipRect(viewport()->rect());
    if(d->hoverIndex().isValid()) {
        auto rowToHighlight = d->hoverIndex().row();
        auto y = rowViewportPosition(rowToHighlight) + offset.y() - 1;
        auto rowh = rowHeight(rowToHighlight) + 1;
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hoverColor.Native());
        painter.drawRect(QRect(-1, y, viewport()->width(), rowh));
        auto span = rowSpan(rowToHighlight, d->hoverIndex().column());
        if(span != 1) {
            auto row = rowToHighlight + 1;
            while(--span) {
                auto y = rowViewportPosition(row) + offset.y() - 1;
                painter.drawRect(-1, y, viewport()->width(), rowHeight(row) + 1);
            }
        }
    }

    if(selectionModel() != nullptr) {
        auto selectedIndexes = selectionModel()->selectedIndexes();
        for(const auto& index : selectedIndexes) {
            if(this->isPersistentEditorOpen(index)) {
                continue;
            }
            auto c = horizontalHeader->visualIndex(index.column());

            auto topIndex = model()->index(index.row() - 1, horizontalHeader->logicalIndex(c));
            auto leftIndex = model()->index(index.row(), horizontalHeader->logicalIndex(c - 1));
            auto rightIndex = model()->index(index.row(), horizontalHeader->logicalIndex(c + 1));
            auto bottomIndex = model()->index(index.row() + 1, horizontalHeader->logicalIndex(c));

            auto vr = visualRect(index);

            painter.setPen(m_selectionColor);
            if(!topIndex.isValid() || !selectionModel()->isSelected(topIndex)) {
                painter.drawLine(vr.x(), vr.y(), vr.right(), vr.y());
            }
            if(!bottomIndex.isValid() || !selectionModel()->isSelected(bottomIndex)) {
                painter.drawLine(vr.x(), vr.bottom(), vr.right(), vr.bottom());
            }
            if(!leftIndex.isValid() || !selectionModel()->isSelected(leftIndex)) {
                painter.drawLine(vr.x(), vr.y(), vr.x(), vr.bottom());
            }
            if(!rightIndex.isValid() || !selectionModel()->isSelected(rightIndex)) {
                painter.drawLine(vr.right(), vr.y(), vr.right(), vr.bottom());
            }
        }
    }

    painter.setPen(old);
    paintDropIndicator(&painter);
}

int sectionSpanEndLogical(const QHeaderView *header, int logical, int span)
{
    int visual = header->visualIndex(logical);
    for (int i = 1; i < span; ) {
        if (++visual >= header->count())
            break;
        logical = header->logicalIndex(visual);
        ++i;
    }
    return logical;
}

int sectionSpanSize(const QHeaderView* header, int logical, int span)
{
    int endLogical = sectionSpanEndLogical(header, logical, span);
    return header->sectionPosition(endLogical)
        - header->sectionPosition(logical)
        + header->sectionSize(endLogical);
}

void WidgetsTableViewBase::drawAndClipSpans(const QRegion &area, QPainter *painter,
                                         const QStyleOptionViewItem &option, QBitArray *drawn,
                                         int firstVisualRow, int lastVisualRow, int firstVisualColumn, int lastVisualColumn, const QColor& gridColor)
{
    Q_D(const WidgetsTableViewBase);
    bool alternateBase = false;
    QRegion region = viewport()->rect();

    QSet<WidgetsTableViewBaseSpan*> visibleSpans;
    bool sectionMoved = verticalHeader()->sectionsMoved() || horizontalHeader()->sectionsMoved();

    if (!sectionMoved) {
        visibleSpans = d->spans().spansInRect(horizontalHeader()->logicalIndex(firstVisualColumn), verticalHeader()->logicalIndex(firstVisualRow),
                                         lastVisualColumn - firstVisualColumn + 1, lastVisualRow - firstVisualRow + 1);
    } else {
        for(int x = firstVisualColumn; x <= lastVisualColumn; x++)
            for(int y = firstVisualRow; y <= lastVisualRow; y++)
                visibleSpans.insert(d->spans().spanAt(x,y));
        visibleSpans.remove(nullptr);
    }

    auto visualSpanRect = [this](const WidgetsTableViewBaseSpan& span) {
        int row = span.top();
        int rowp = verticalHeader()->sectionViewportPosition(row);
        int rowh = sectionSpanSize(verticalHeader(), row, span.height());
        // horizontal
        int column = span.left();
        int colw = sectionSpanSize(horizontalHeader(), row, span.width());
        if (isRightToLeft())
            column = span.right();
        int colp = horizontalHeader()->sectionViewportPosition(column);

        const int i = showGrid() ? 1 : 0;
        if (isRightToLeft())
            return QRect(colp + i, rowp, colw - i, rowh - i);
        return QRect(colp, rowp, colw - i, rowh - i);
    };

    for (WidgetsTableViewBaseSpan *span : qAsConst(visibleSpans)) {
        int row = span->top();
        int col = span->left();
        QModelIndex index = model()->index(row, col, rootIndex());
        if (!index.isValid())
            continue;
        QRect rect = visualSpanRect(*span);
        rect.translate(d->scrollDelayOffset());
        if (!area.intersects(rect))
            continue;
        QStyleOptionViewItem opt = option;
        opt.rect = rect;
        alternateBase = (span->top() & 1);
        opt.features.setFlag(QStyleOptionViewItem::Alternate, alternateBase);
        drawCell(painter, opt, index);
        painter->setPen(gridColor);
        painter->drawLine(opt.rect.right(), opt.rect.top(), opt.rect.right(), opt.rect.bottom());
        painter->drawLine(opt.rect.left(), opt.rect.top(), opt.rect.left(), opt.rect.bottom());
        painter->drawLine(opt.rect.left(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
//        if (showGrid()) {
//            // adjust the clip rect to be able to paint the top & left grid lines
//            // if the headers are not visible, see paintEvent()
//            if (horizontalHeader()->visualIndex(row) == 0)
//                rect.setTop(rect.top() + 1);
//            if (verticalHeader()->visualIndex(row) == 0) {
//                if (isLeftToRight())
//                    rect.setLeft(rect.left() + 1);
//                else
//                    rect.setRight(rect.right() - 1);
//            }
//        }
        region -= rect;
        for (int r = span->top(); r <= span->bottom(); ++r) {
            const int vr = verticalHeader()->visualIndex(r);
            if (vr < firstVisualRow || vr > lastVisualRow)
                continue;
            for (int c = span->left(); c <= span->right(); ++c) {
                const int vc = horizontalHeader()->visualIndex(c);
                if (vc < firstVisualColumn  || vc > lastVisualColumn)
                    continue;
                drawn->setBit((vr - firstVisualRow) * (lastVisualColumn - firstVisualColumn + 1)
                             + vc - firstVisualColumn);
            }
        }

    }
    painter->setClipRegion(region);
}

void WidgetsTableViewBase::drawCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_D(WidgetsTableViewBase);
    QStyleOptionViewItem opt = option;

    //        if (selectionModel() && selectionModel()->isSelected(index))
    //            opt.state |= QStyle::State_Selected;
    //        if (index == d->hoverIndex())
    //            opt.state |= QStyle::State_MouseOver;
    if (option.state & QStyle::State_Enabled) {
        QPalette::ColorGroup cg;
        if ((model()->flags(index) & Qt::ItemIsEditable) == 0) {
            opt.state &= ~QStyle::State_Enabled;
            cg = QPalette::Disabled;
        } else {
            cg = QPalette::Normal;
        }
        opt.palette.setCurrentColorGroup(cg);
    }

    if (index == currentIndex()) {
        const bool focus = (hasFocus() || viewport()->hasFocus()) && currentIndex().isValid();
        if (focus)
            opt.state |= QStyle::State_HasFocus;
    }

    itemDelegate(index)->paint(painter, opt, index);

    static auto drawTriangle = [](QPainter* painter, const QStyleOptionViewItem& option, const QColor& c) {
        QPolygonF polygon;
        const auto tl = option.rect.topLeft();
        const auto br = option.rect.bottomRight();
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        polygon.append(QPointF(br.x() - 10.0, tl.y()));
        polygon.append(QPointF(br.x(), tl.y()));
        polygon.append(QPointF(br.x(), tl.y() + 10.0));
        painter->drawPolygon(polygon);
    };

    switch (index.data(FieldHasErrorRole).toInt()) {
    case 1: drawTriangle(painter, option, SharedSettings::GetInstance().StyleSettings.DefaultRedColor); break;
    case 2: drawTriangle(painter, option, SharedSettings::GetInstance().StyleSettings.DefaultOrangeColor); break;
    default: break;
    }
}

void WidgetsTableViewBase::paintDropIndicator(QPainter* painter)
{
    Q_D(WidgetsTableViewBase);
    if (showDropIndicator() && state() == QAbstractItemView::DraggingState
        #ifndef QT_NO_CURSOR
            && viewport()->cursor().shape() != Qt::ForbiddenCursor
        #endif
            ) {
        QStyleOption opt;
        opt.init(this);
        opt.rect = d->dropIndicatorRect();
        style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemDrop, &opt, painter, this);
    }
}

void WidgetsTableViewBase::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(WidgetsTableViewBase);
    auto prevHover = d->hoverIndex();
    Super::mouseMoveEvent(event);
    if(prevHover != d->hoverIndex()) {
        update();
    }
}

