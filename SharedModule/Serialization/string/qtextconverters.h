#ifndef QSTRINGCONVERTERS_H
#define QSTRINGCONVERTERS_H

#include <QUrl>

template <>
struct TextConverter<QUrl>
{
    using value_type = QUrl;
    static QString ToText(const value_type& value)
    {
        return value.toString();
    }

    static value_type FromText(const QString& string)
    {
        return value_type(string);
    }
};

template <class T>
struct TextConverter<QList<T>>
{
    using value_type = QList<T>;
    static QString ToText(const value_type& value)
    {
        QString result;
        result += "[";
        for(const auto& subValue : value) {
            result += "(" + TextConverter<T>::ToText(subValue) + ")";
        }
        result += "]";
        return result;
    }

    static value_type FromText(const QString& string)
    {
        static QRegExp regExp(R"(\(([^\)]+)\))");
        qint32 pos = 0;
        value_type result;
        while((pos = regExp.indexIn(string, pos)) != -1) {
            result.append(TextConverter<T>::FromText(regExp.cap(1)));
            pos += regExp.matchedLength();
        }
        return result;
    }
};

template <>
struct TextConverter<QString>
{
    using value_type = QString;
    static QString ToText(const value_type& value)
    {
        return value;
    }

    static value_type FromText(const QString& string)
    {
        return string;
    }
};

template <>
struct TextConverter<qint32>
{
    using value_type = qint32;
    static QString ToText(const value_type& value)
    {
        return QString::number(value);
    }

    static value_type FromText(const QString& string)
    {
        return string.toInt();
    }
};


template <>
struct TextConverter<Name>
{
    using value_type = Name;
    static QString ToText(const value_type& value)
    {
        return value.AsString();
    }

    static value_type FromText(const QString& string)
    {
        return Name(string);
    }
};

template <class Key, class Value>
struct TextConverter<QHash<Key, Value>>
{
    using value_type = QHash<Key, Value>;
    static QString ToText(const value_type& value)
    {
        QString result;
        for(auto it = value.begin(), e = value.end(); it != e; it++) {
            result += "{(" + TextConverter<Key>::ToText(it.key()) + ")|(";
            result += TextConverter<Value>::ToText(it.value()) + ")}";
        }
        return result;
    }

    static value_type FromText(const QString& string)
    {
        static QRegExp regExp(R"(\{\(([^\|]+)\)\|\(([^\}]+)\)\})");
        qint32 pos = 0;
        value_type result;
        while((pos = regExp.indexIn(string, pos)) != -1) {
            result.insert(TextConverter<Key>::FromText(regExp.cap(1)), TextConverter<Value>::FromText(regExp.cap(2)));
            pos += regExp.matchedLength();
        }

        return result;
    }
};

template <typename Key>
struct TextConverter<QSet<Key>>
{
    using value_type = QSet<Key>;
    static QString ToText(const value_type& value)
    {
        QString result;
        for(const auto& key : value) {
            result += "(" + TextConverter<Key>::ToText(key) + ")";
        }
        return result;
    }

    static value_type FromText(const QString& string)
    {
        static QRegExp regExp(R"(\(([^\)]+)\))");
        qint32 pos = 0;
        value_type result;
        while((pos = regExp.indexIn(string, pos)) != -1) {
            result.insert(TextConverter<Key>::FromText(regExp.cap(1)));
            pos += regExp.matchedLength();
        }

        return result;
    }
};

#endif // QSTRINGCONVERTERS_H
