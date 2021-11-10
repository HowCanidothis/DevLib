#ifndef WIDGETSSTANDARTTABLEATTACHMENT_H
#define WIDGETSSTANDARTTABLEATTACHMENT_H


class WidgetsStandartTableAttachment
{
public:
    static void Attach(class QTableView* tableView, const QSet<qint32>& ignoreColumns = {});
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
