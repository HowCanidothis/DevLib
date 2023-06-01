#include "widgetsdeclarations.h"

#include <QLabel>

namespace WidgetProperties {
IMPLEMENT_GLOBAL_CHAR_1(ExtraFieldsCount);
IMPLEMENT_GLOBAL_CHAR(InnerSplitter, inner);
IMPLEMENT_GLOBAL_CHAR_1(Footer)
}

DescCustomDialogParams& DescCustomDialogParams::FillWithText(const QString& text)
{
    Q_ASSERT(View == nullptr);
    View = new QLabel(text);
    return *this;
}
