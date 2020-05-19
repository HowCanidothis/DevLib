#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include <QString>

template <class T>
struct TextConverter
{    
    static QString ToText(const T& value)
    {
        return value.ToString();
    }
    static T FromText(const QString& string)
    {
        return T::FromString(string);
    }
};

template<>
struct TextConverter<double>
{
    static QString ToText(double value)
    {
        return QString::number(value, 'f', 6);
    }
    static double FromText(const QString& string)
    {
        return string.toDouble();
    }
};

#endif // STRINGCONVERTER_H
