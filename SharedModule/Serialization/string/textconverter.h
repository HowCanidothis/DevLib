#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include <QString>

#include "textconvertercontext.h"

template <class T>
struct TextConverter
{    
    static QString ToText(const T& value, const TextConverterContext& context)
    {
        return value.ToString();
    }
    static T FromText(const QString& string)
    {
        T result;
        result.FromString(string);
        return result;
    }
};

struct TextHelper
{
    template<class T> static QString ToText(const T& value, const TextConverterContext& context = TextConverterContext())
    {
        return TextConverter<T>::ToText(value, context);
    }

    template<class T> static T FromText(const QString& value)
    {
        return TextConverter<T>::FromText(value);
    }
};

#endif // STRINGCONVERTER_H
