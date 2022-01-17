#include "widgetsdraganddropheader.h"

#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include <QTableView>
#include <QMouseEvent>

#include <SharedModule/External/external.hpp>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDragAndDropHeader::WidgetsDragAndDropHeader(QTableView* parent, Qt::Orientation orientation)
    : Super(orientation, parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSectionsClickable(true);
    setSortIndicatorShown(true);
    setHighlightSections(true);
}

QMenu* WidgetsDragAndDropHeader::CreateShowColumsMenu(QMenu* parent, const DescColumnsParams& params)
{
    auto* result = createPreventedFromClosingMenu(orientation() == Qt::Horizontal ? tr("Show Columns") : tr("Show Rows"), parent);
    connect(result, &QMenu::aboutToShow, [result, params, this]{
        result->clear();
        QTableView* table = qobject_cast<QTableView*> (parentWidget());
        Q_ASSERT(table != nullptr);

        auto* model = table->model();
        auto count = orientation() == Qt::Horizontal ? model->columnCount() : model->rowCount();
        for(int i=0; i<count; ++i){
            auto foundIt = params.ColumnsParams.find(i);
            if(foundIt != params.ColumnsParams.end()){
                setSectionHidden(i, !foundIt->Visible);
                if(!foundIt->CanBeHidden) {
                    continue;
                }
            }
            auto title = model->headerData(i, orientation()).toString();
            auto* action = createCheckboxAction(title, !isSectionHidden(i), [this, i](bool checked){
                setSectionHidden(i, !checked);
            }, result);
            action->setProperty("index", i);
        }
        auto oldActions = result->actions();
        createAction(tr("Switch"), [oldActions]{
            for(auto* action : oldActions) {
                action->trigger();
            }
        }, result);
        for(auto* action : result->actions()){
            auto index = action->property("index").toInt();
            if(0 <= index && index < count){
                action->setChecked(!isSectionHidden(index));
            }
        }
    });
    if(!params.ColumnsParams.isEmpty()) {
        WidgetsAttachment::Attach(this, [this,params](QObject*, QEvent* event){
            if(event->type() == QEvent::Show) {
                for(auto it(params.ColumnsParams.cbegin()), e(params.ColumnsParams.cend()); it != e; it++){
                    setSectionHidden(it.key(), !it.value().Visible);
                }
            }
            return false;
        });
    }

    return result;
}

void WidgetsDragAndDropHeader::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton && cursor().shape() == Qt::ArrowCursor)
    {
        int logicIndex = logicalIndexAt(event->pos());
        int visibleIndex = visualIndex(logicIndex);

        auto* mimeData = new QMimeData();
        mimeData->setText(QString::number(visibleIndex));

        auto* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setDragCursor(pixmapForDrag(logicIndex), Qt::DropAction::MoveAction);
        drag->exec(Qt::MoveAction);
    } else {
        Super::mouseMoveEvent(event);
    }
}

void WidgetsDragAndDropHeader::dragMoveEvent(QDragMoveEvent* event)
{
    int indexFrom = event->mimeData()->text().toInt();
    auto loigicIndex = logicalIndexAt(event->pos());
    int indexTo = visualIndex(loigicIndex);

    if(indexFrom != indexTo && indexFrom >= 0 && indexTo >= 0){
        event->accept();
    } else {
        event->ignore();
    }
}

void WidgetsDragAndDropHeader::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}

void WidgetsDragAndDropHeader::dropEvent(QDropEvent* event)
{
    int indexFrom = event->mimeData()->text().toInt();
    int indexTo = visualIndex(logicalIndexAt(event->pos()));
    moveSection(indexFrom, indexTo);
}

QPixmap WidgetsDragAndDropHeader::pixmapForDrag(const int index) const
{
    QTableView* table = qobject_cast<QTableView*> (this->parentWidget());
    Q_ASSERT(table != nullptr);

    QRect rect;
    switch(orientation()) {
    case Qt::Vertical: {
        int fullWidth = width();
        for (int iRow = 0; iRow < 5 && iRow < table->model()->columnCount(); ++iRow)
        {
            fullWidth += table->rowHeight(iRow);
        }

        fullWidth = qMin(table->width(), fullWidth);
        auto x = table->columnViewportPosition(0);
        auto y = table->rowViewportPosition(index) + table->horizontalHeader()->height();
        rect = QRect(x, y, fullWidth, table->rowHeight(index));
        break;
    }
    case Qt::Horizontal:{
        int fullHeight = height();
        for (int iRow = 0; iRow < 5 && iRow < table->model()->rowCount(); ++iRow)
        {
            fullHeight += table->rowHeight(iRow);
        }

        fullHeight = qMin(table->height(), fullHeight);
        auto x = table->columnViewportPosition(index) + table->verticalHeader()->width();
        auto y = table->rowViewportPosition(0);
        rect = QRect(x, y, table->columnWidth(index), fullHeight);
        break;
    }
    }

    QPixmap pixmap (rect.size());
    table->render(&pixmap, QPoint(), QRegion(rect));
    return pixmap;
}
