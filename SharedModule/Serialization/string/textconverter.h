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

#endif // STRINGCONVERTER_H
