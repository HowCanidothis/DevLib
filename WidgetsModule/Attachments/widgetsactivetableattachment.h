#ifndef WIDGETSACTIVETABLEATTACHMENT_H
#define WIDGETSACTIVETABLEATTACHMENT_H

#include <PropertiesModule/internal.hpp>

class WidgetsActiveTableViewAttachment : public QObject
{
    WidgetsActiveTableViewAttachment();
public:
    LocalPropertyInt SelectedRowsCount;
    LocalPropertyBool HasSelection;
    LocalPropertyPtr<QTableView> ActiveTable;

    static WidgetsActiveTableViewAttachment* GetInstance();
    static void Attach(class QTableView* tableView);

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void updateActiveTableView(QTableView* tableView);
};

#endif // WIDGETSACTIVETABLEATTACHMENT_H
