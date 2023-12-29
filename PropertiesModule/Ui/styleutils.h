#ifndef STYLEUTILS_H
#define STYLEUTILS_H

#include <SharedModule/declarations.h>

namespace WidgetProperties {
DECLARE_GLOBAL_CHAR(ForceDisabled)
}

namespace StyleUtils {
    void UpdateStyle(class QWidget* target, bool recursive = false);
    void ApplyStyleProperty(const char* propertyName, class QWidget* target, const QVariant& value, bool recursive = false);
}

#endif // STYLEUTILS_H
