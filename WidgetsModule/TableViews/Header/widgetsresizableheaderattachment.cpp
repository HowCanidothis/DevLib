#include "widgetsresizableheaderattachment.h"

#include <QTableView>
#include <QMenu>

#include <SharedModule/hacks.h>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsResizableHeaderAttachment::WidgetsResizableHeaderAttachment(Qt::Orientation orientation, QTableView* parent, const DescTableViewParams& params)
    : Super(orientation, parent)
{
    qint32 alignment = Qt::AlignCenter | Qt::TextWordWrap;
    setDefaultAlignment((Qt::Alignment)alignment);
    setStretchLastSection(params.StretchLastSection);
    setSectionsMovable(params.SectionsMovable);
    setSectionsClickable(params.SectionsClickable);
    setHighlightSections(params.HighlightSections);

    setDropIndicatorShown(params.DropIndicatorShown);
    setSortIndicatorShown(params.SortIndicatorShown);
}

QMenu* WidgetsResizableHeaderAttachment::CreateShowColumsMenu(QHeaderView* hv, const DescTableViewParams& params)
{
    QTableView* table = qobject_cast<QTableView*> (hv->parentWidget());
    Q_ASSERT(table != nullptr);
    auto* result = MenuWrapper(table).AddPreventedFromClosingMenu(hv->orientation() == Qt::Horizontal ? tr("Show Columns") : tr("Show Rows"));
    connect(result, &QMenu::aboutToShow, [table, result, params, hv]{
        result->clear();
        MenuWrapper wrapper(result);

        auto* model = table->model();
        for(int i=0; i < hv->count(); ++i){
            auto foundIt = params.ColumnsParams.find(i);
            if(foundIt != params.ColumnsParams.end()){
                if(!foundIt->CanBeHidden) {
                    continue;
                }
            }
            auto title = model->headerData(i, hv->orientation()).toString();
            wrapper.AddCheckboxAction(title, !hv->isSectionHidden(i), [hv, i](bool checked){
                hv->setSectionHidden(i, !checked);
            });
        }
        auto oldActions = result->actions();
        wrapper.AddAction(tr("Switch"), [oldActions]{
            for(auto* action : oldActions) {
                action->trigger();
            }
        });
    });
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

