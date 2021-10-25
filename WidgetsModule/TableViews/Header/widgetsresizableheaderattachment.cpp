#include "widgetsresizableheaderattachment.h"

#include <QTableView>
#include <SharedModule/hacks.h>

WidgetsResizableHeaderAttachment::WidgetsResizableHeaderAttachment(QTableView* parent)
    : Super(parent)
{
    qint32 alignment = Qt::AlignCenter | Qt::TextWordWrap;
    setDefaultAlignment((Qt::Alignment)alignment);
    setStretchLastSection(true);
}

QSize WidgetsResizableHeaderAttachment::sectionSizeFromContents(int logicalIndex) const
{
    if(model() != nullptr && model()->rowCount() != 0) {
        auto headerText = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        auto options = viewOptions();
        auto metrics = QFontMetrics(options.font);
        auto maxWidth = sectionSize(logicalIndex);
        int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, &options, this);
        margin += style()->pixelMetric(QStyle::PM_HeaderGripMargin, &options, this);
        auto rect = metrics.boundingRect(QRect(margin, margin, maxWidth - margin, 5000), defaultAlignment(),
                                    headerText);
        return rect.size() + QSize(margin * 2, margin * 2);
    }

    return Super::sectionSizeFromContents(logicalIndex);
}

void WidgetsResizableHeaderAttachment::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    Super::paintSection(painter, rect, logicalIndex);
}

