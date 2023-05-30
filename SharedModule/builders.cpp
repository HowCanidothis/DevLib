#include "builders.h"

QString StringBuilder::XMLCreateStyleColorized(const QString& text, const QColor& color)
{
    return QString("<a style=\"color: %1\">%2</a>").arg(color.name(), text);
}

QString StringBuilder::XMLCreateHyperlink(const QString& text, const FAction* handler, const QColor& color)
{
    return QString("<a href=\"%1\" style=\"color: %2\">%3</a>").arg(QString::number((size_t)handler), color.name(), text);
}

StringBuilder& StringBuilder::XMLAddEnumerated(const QString& text)
{
    *this += "<li>";
    *this += text;
    *this += "</li>";
    return *this;
}
