#ifndef WIDGETSRESIZABLEHEADERATTACHMENT_H
#define WIDGETSRESIZABLEHEADERATTACHMENT_H

#include "widgetsdraganddropheader.h"

class WidgetsResizableHeaderAttachment : public WidgetsDragAndDropHeader
{
    using Super = WidgetsDragAndDropHeader;
public:
    WidgetsResizableHeaderAttachment(class QTableView* parent);

    // QHeaderView interface
protected:
    QSize sectionSizeFromContents(int logicalIndex) const override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
};

#endif // WIDGETSRESIZABLEHEADERATTACHMENT_H
