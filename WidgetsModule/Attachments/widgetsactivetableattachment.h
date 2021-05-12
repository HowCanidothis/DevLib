#ifndef WIDGETSACTIVETABLEATTACHMENT_H
#define WIDGETSACTIVETABLEATTACHMENT_H

#include <PropertiesModule/internal.hpp>

class WidgetsActiveTableViewAttachment : public QObject
{
    WidgetsActiveTableViewAttachment();
public:
    static void Attach(class QTableView* tableView);

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    static WidgetsActiveTableViewAttachment* instance();

private:
    PointerPropertyPtr<QTableView> m_activeTable;
    BoolPropertyPtr m_hasSelection;
};

#endif // WIDGETSACTIVETABLEATTACHMENT_H
