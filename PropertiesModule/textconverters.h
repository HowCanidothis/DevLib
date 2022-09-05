#ifndef PROPERTIES_TEXTCONVERTERS_H
#define PROPERTIES_TEXTCONVERTERS_H

#include "localproperty.h"

template<typename T>
struct TextConverter<LocalProperty<T>>
{
    static QString ToText(const LocalProperty<T>& value, const TextConverterContext& context)
    {
        return TextConverter<T>::ToText(value, context);
    }
};

template<typename T>
struct TextConverter<LocalPropertyOptional<T>>
{
    static QString ToText(const LocalPropertyOptional<T>& value, const TextConverterContext& context)
    {
        if(!value.IsValid) {
            return QString();
        }
        return TextConverter<T>::ToText(value.Value, context);
    }
};

template<typename T>
struct TextConverter<LocalPropertyLimitedDecimal<T>>
{
    static QString ToText(const LocalPropertyLimitedDecimal<T>& value, const TextConverterContext& context)
    {
        return TextConverter<T>::ToText(value, context);
    }
};

#endif // TEXTCONVERTERS_H
