#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

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

#endif // STRINGCONVERTER_H
