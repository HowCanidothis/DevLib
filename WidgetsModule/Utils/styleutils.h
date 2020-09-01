#ifndef STYLE_UTILS_H
#define STYLE_UTILS_H

#include <QVariant>

class StyleUtils
{
    StyleUtils();
public:

    static void ApplyStyleProperty(const char* propertyName, class QWidget* target, const QVariant& value);
};

#endif // POLISHER_H
