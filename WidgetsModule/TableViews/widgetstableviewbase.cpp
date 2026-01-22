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

class WidgetsTableViewBasePrivate
{
public:
    const QPoint& scrollDelayOffset() const { return *((QPoint*)((size_t)this + 0x380)); }
//    qint32 top() const { return *((qint32*)((size_t)this + 0x244)); }
    const QPersistentModelIndex& hoverIndex() const { return *((QPersistentModelIndex*)((size_t)this + 0x310)); }
    const QRect& dropIndicatorRect() const { return *((QRect*)((size_t)this + 0x31c)); }
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

    //        if (d->hasSpans()) {
    //            d->drawAndClipSpans(region, &painter, option, &drawn,
    //                                 firstVisualRow, lastVisualRow, firstVisualColumn, lastVisualColumn);
    //        }

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

    if(d->hoverIndex().isValid()) {
        auto rowToHighlight = d->hoverIndex().row();
        auto y = rowViewportPosition(rowToHighlight) + offset.y() - 1;
        auto rowh = rowHeight(rowToHighlight) + 1;
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hoverColor.Native());
        painter.drawRect(QRect(-1, y, viewport()->width(), rowh));
    }

    if(selectionModel() != nullptr && !selectionModel()->selectedIndexes().isEmpty()) {
        for(const auto& index : selectionModel()->selectedIndexes()) {
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

