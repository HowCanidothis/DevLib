#include "styleutils.h"

#include <QWidget>
#include <QStyle>

namespace WidgetProperties {
IMPLEMENT_GLOBAL_CHAR_1(ForceDisabled)
}

void StyleUtils::UpdateStyle(class QWidget* target, bool recursive)
{
    auto* style = target->style();
    style->unpolish(target);
    style->polish(target);
    if(recursive) {
        auto childWidgets = target->findChildren<QWidget*>();
        for(auto* widget : childWidgets) {
            auto* style = widget->style();
            style->unpolish(widget);
            style->polish(widget);
        }
    }
}

void StyleUtils::ApplyStyleProperty(const char* propertyName, QWidget* target, const QVariant& value, bool recursive)
{
    target->setProperty(propertyName, value);
    UpdateStyle(target, recursive);
}
