#ifndef WIDGETSSTANDARTTABLEATTACHMENT_H
#define WIDGETSSTANDARTTABLEATTACHMENT_H


class WidgetsStandartTableAttachment
{
public:
    static void Attach(class QTableView* tableView);
    static void AttachWithShowHide(class QTableView* tableView, const QSet<qint32>& ignorColumns = {});
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
