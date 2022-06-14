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

template<class T>
QString TextConverterToText(const T& value, const TextConverterContext& context = TextConverterContext())
{
    return TextConverter<T>::ToText(value, context);
}

template<class T>
void TextConverterFromText(const QString& text, T& value)
{
    value = TextConverter<T>::FromText(text);
}


#endif // STRINGCONVERTER_H
