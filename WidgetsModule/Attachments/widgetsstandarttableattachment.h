#ifndef WIDGETSSTANDARTTABLEATTACHMENT_H
#define WIDGETSSTANDARTTABLEATTACHMENT_H

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsStandartTableAttachment
{
public:
    static void Attach(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
