#ifndef STYLE_UTILS_H
#define STYLE_UTILS_H

#include <QVariant>

class StyleUtils
{
    StyleUtils();
public:

    static void UpdateStyle(class QWidget* target, bool recursive = false);
    static void ApplyStyleProperty(const char* propertyName, class QWidget* target, const QVariant& value, bool recursive = false);
    // Qt bug? Widgets which don't participate at layout system does not update their padding
    // SizeAdjuster fixes this issue
    static void InstallSizeAdjuster(QWidget* widget);
};

#endif // POLISHER_H
