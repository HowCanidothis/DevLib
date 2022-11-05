#include "widgetsdeclarations.h"

#include <QLabel>

const char* ExtraFieldsCountPropertyName = "ExtraFieldsCount";

DescCustomDialogParams& DescCustomDialogParams::FillWithText(const QString& text)
{
    Q_ASSERT(View == nullptr);
    View = new QLabel(text);
    return *this;
}
