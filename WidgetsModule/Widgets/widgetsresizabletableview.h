#ifndef WIDGETSRESIZABLETABLEVIEW_H
#define WIDGETSRESIZABLETABLEVIEW_H

#include <QTableView>
#include <SharedModule/internal.hpp>

class WidgetsResizableTableView: public QTableView
{
    using Super = QTableView;
public:
    WidgetsResizableTableView(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel *model) override;
    QSize sizeHint() const override;

private:
    QtLambdaConnections m_connections;
};


#endif // WIDGETSRESIZABLETABLEVIEW_H
