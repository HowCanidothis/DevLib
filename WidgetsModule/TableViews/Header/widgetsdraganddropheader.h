#ifndef WIDGETSDRAGANDDROPHEADER_H
#define WIDGETSDRAGANDDROPHEADER_H

#include <QPixmap>
#include <QHeaderView>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsDragAndDropHeader: public QHeaderView
{
    Q_OBJECT
    using Super = QHeaderView;
public:
    WidgetsDragAndDropHeader(class QTableView* parent = nullptr);

    class QMenu* CreateShowColumsMenu(QMenu* parent, const DescColumnsParams& ignorColumns = DescColumnsParams());
protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QPixmap pixmapForDrag(const int columnIndex) const;
};

#endif // WIDGETSDRAGANDDROPHEADER_H
