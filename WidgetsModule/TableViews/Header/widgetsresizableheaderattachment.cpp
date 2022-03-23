#include "widgetsresizableheaderattachment.h"

#include <QTableView>
#include <QMenu>

#include <SharedModule/hacks.h>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsResizableHeaderAttachment::WidgetsResizableHeaderAttachment(Qt::Orientation orientation, QTableView* parent)
    : Super(orientation, parent)
{
    qint32 alignment = Qt::AlignCenter | Qt::TextWordWrap;
    setDefaultAlignment((Qt::Alignment)alignment);
    setStretchLastSection(true);
    setSectionsMovable(true);
    setSectionsClickable(true);
    setHighlightSections(true);

    setDropIndicatorShown(true);
    setSortIndicatorShown(true);
}

QMenu* WidgetsResizableHeaderAttachment::CreateShowColumsMenu(QMenu* parent, const DescColumnsParams& params)
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

QSize WidgetsResizableHeaderAttachment::sectionSizeFromContents(int logicalIndex) const
{
    if(model() != nullptr && model()->rowCount() != 0) {
        auto headerText = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        auto options = viewOptions();
        auto metrics = QFontMetrics(options.font);
        int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, &options, this);
        margin += style()->pixelMetric(QStyle::PM_HeaderGripMargin, &options, this);
        qint32 maxWidth;
        if(orientation() == Qt::Horizontal) {
            maxWidth = sectionSize(logicalIndex);
        } else {
            maxWidth = width();
        }
        auto rect = metrics.boundingRect(QRect(margin, margin, maxWidth - margin, 5000), defaultAlignment(), headerText);
        return rect.size() + QSize(margin * 2, margin * 2);
    }

    return Super::sectionSizeFromContents(logicalIndex);
}

void WidgetsResizableHeaderAttachment::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    Super::paintSection(painter, rect, logicalIndex);
}

