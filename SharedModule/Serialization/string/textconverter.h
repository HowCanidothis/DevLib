#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

template <class T>
struct TextConverter
{    
    static QString ToText(const T& value);
    static T FromText(const QString& string);
};

#endif // STRINGCONVERTER_H
