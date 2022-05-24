#ifndef WIDGETSRESIZABLEHEADERATTACHMENT_H
#define WIDGETSRESIZABLEHEADERATTACHMENT_H

#include <QHeaderView>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsResizableHeaderAttachment : public QHeaderView
{
    Q_OBJECT
    using Super = QHeaderView;
public:
    WidgetsResizableHeaderAttachment(Qt::Orientation orientation, class QTableView* parent);

    class QMenu* CreateShowColumsMenu(const DescColumnsParams& ignorColumns = DescColumnsParams());
    // QHeaderView interface
protected:
    QSize sectionSizeFromContents(int logicalIndex) const override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
};

#endif // WIDGETSRESIZABLEHEADERATTACHMENT_H
