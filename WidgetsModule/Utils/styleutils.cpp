#include "styleutils.h"

#include <QStyle>
#include <QWidget>

StyleUtils::StyleUtils()
{

}


void StyleUtils::ApplyStyleProperty(const char* propertyName, QWidget* target, const QVariant& value)
{
    target->setProperty(propertyName, value);
    auto* style = target->style();
    style->unpolish(target);
    style->polish(target);
}
