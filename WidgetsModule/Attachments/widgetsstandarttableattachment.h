#ifndef WIDGETSSTANDARTTABLEATTACHMENT_H
#define WIDGETSSTANDARTTABLEATTACHMENT_H

#include "WidgetsModule/widgetsdeclarations.h"

class QHeaderView;
class WidgetsStandartTableAttachment
{
public:
    static QHeaderView* AttachHorizontal(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
    static QHeaderView* AttachVertical(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
