#ifndef QSTRINGCONVERTERS_H
#define QSTRINGCONVERTERS_H

#include <QUrl>
#include <QRegularExpression>
#ifdef QT_GUI_LIB
#include <QColor>
#include <QMatrix4x4>
#endif

#include "textconvertercontext.h"

template <typename T>
struct TextConverter<std::optional<T>>
{
    using value_type = std::optional<T>;
    static QString ToText(const value_type& value, const TextConverterContext& context)
    {
        return value.has_value() ? TextConverter<T>::ToText(value.value(), context) : QString();
    }

    static value_type FromText(const QString& string)
    {
        return string.isEmpty() ? std::nullopt : std::make_optional(TextConverter<T>::FromText(string));
    }

    static value_type FromText(const QStringView& string)
    {
        return string.isEmpty() ? std::nullopt : std::make_optional(TextConverter<T>::FromText(string));
    }
};

template <>
struct TextConverter<QUrl>
{
    using value_type = QUrl;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.toString();
    }

    static value_type FromText(const QString& string)
    {
        return value_type(string);
    }
};

template <>
struct TextConverter<QByteArray>
{
    using value_type = QByteArray;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value;
    }

    static value_type FromText(const QString& string)
    {
        return string.toLatin1();
    }
};

template <>
struct TextConverter<QDateTime>
{
    using value_type = QDateTime;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.toString();
    }

    static value_type FromText(const QString& string)
    {
        return QDateTime::fromString(string);
    }
};

template <>
struct TextConverter<QDate>
{
    using value_type = QDate;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.toString();
    }

    static value_type FromText(const QString& string)
    {
        return QDate::fromString(string);
    }
};

template <>
struct TextConverter<QTime>
{
    using value_type = QTime;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.toString();
    }

    static value_type FromText(const QString& string)
    {
        return QTime::fromString(string);
    }
};

template <class T>
struct TextConverter<QList<T>>
{
    using value_type = QList<T>;
    static QString ToText(const value_type& value, const TextConverterContext& context)
    {
        QString result;
        result += "[";
        for(const auto& subValue : value) {
            result += "(" + TextConverter<T>::ToText(subValue, context) + ")";
        }
        result += "]";
        return result;
    }

    static value_type FromText(const QString& string)
    {
        thread_local static QRegularExpression regExp(R"(\(([^\)]+)\))");
        value_type result;
        auto it = regExp.globalMatch(string, 0);
        while(it.hasNext()) {
            auto next = it.next();
            result.append(TextConverter<T>::FromText(next.captured(1)));
        }
        return result;
    }
};

template <>
struct TextConverter<QString>
{
    using value_type = QString;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value;
    }

    static value_type FromText(const QString& string)
    {
        return string;
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toString();
    }
};

template <>
struct TextConverter<QLocale>
{
    using value_type = QLocale;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.name();
    }

    static value_type FromText(const QString& string)
    {
        return QLocale(string);
    }
};

template <>
struct TextConverter<QVariant>
{
    using value_type = QVariant;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.toString();
    }

    static value_type FromText(const QString& string)
    {
        return QVariant(string);
    }
};

template <>
struct TextConverter<qint32>
{
    using value_type = qint32;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return QString::number(value);
    }

    static value_type FromText(const QString& string)
    {
        return string.toDouble();
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toDouble();
    }
};

template <>
struct TextConverter<qint64>
{
    using value_type = qint64;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return QString::number(value);
    }

    static value_type FromText(const QString& string)
    {
        return string.toDouble();
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toDouble();
    }
};

template <>
struct TextConverter<size_t>
{
    using value_type = size_t;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return QString::number(value);
    }

    static value_type FromText(const QString& string)
    {
        return string.toDouble();
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toDouble();
    }
};


template<>
struct TextConverter<double>
{
    using value_type = double;
    static QString ToText(value_type value, const TextConverterContext& context)
    {
        return QString::number(value, 'f', context.DoublePrecision);
    }
    static value_type FromText(const QString& string)
    {
        return string.toDouble();
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toDouble();
    }
};

template<>
struct TextConverter<float>
{
    using value_type = float;
    static QString ToText(value_type value, const TextConverterContext& context)
    {
        return QString::number(value, 'f', context.FloatPrecision);
    }
    static value_type FromText(const QString& string)
    {
        return string.toFloat();
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toDouble();
    }
};

template<>
struct TextConverter<bool>
{
    using value_type = bool;
    static QString ToText(value_type value, const TextConverterContext& )
    {
        return QString::number((qint32)value);
    }
    static value_type FromText(const QString& string)
    {
        return string.toInt();
    }

    static value_type FromText(const QStringView& string)
    {
        return string.toInt();
    }
};

#ifdef QT_GUI_LIB
template <>
struct TextConverter<QColor>
{
    using value_type = QColor;
    static QString ToText(const value_type& value, const TextConverterContext& )
    {
        return value.name();
    }

    static value_type FromText(const QString& string)
    {
        return QColor(string);
    }
};

template <>
struct TextConverter<QMatrix4x4>
{
    using value_type = QMatrix4x4;
    static QString ToText(const value_type& value, const TextConverterContext& context)
    {
        QString result;
        result += "[";
        const auto* data = value.constData();
        for(qint32 row(0); row < 4; row++) {
            for(qint32 col(0); col < 4; col++) {
                result += "(" + TextConverter<float>::ToText(data[col * 4 + row], context) + ")";
            }
        }
        result += "]";
        return result;
    }

    static value_type FromText(const QString& string)
    {
        thread_local static QRegularExpression regExp(R"(\(([^\)]+)\))");
        float values[16];

        qint32 index = 0;
        auto it = regExp.globalMatch(string);
        while(it.hasNext()) {
            values[index] = TextConverter<float>::FromText(it.next().captured(1));
            index++;
        }

        auto result = QMatrix4x4(values);
        result.optimize();
        return result;
    }
};

#endif

template <>
struct TextConverter<Name>
{
    using value_type = Name;
    static QString ToText(const value_type& value, const TextConverterContext& )
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
    static QString ToText(const value_type& value, const TextConverterContext& context)
    {
        QString result;
        for(auto it = value.begin(), e = value.end(); it != e; it++) {
            result += "{(" + TextConverter<Key>::ToText(it.key(), context) + ")|(";
            result += TextConverter<Value>::ToText(it.value(), context) + ")}";
        }
        return result;
    }

    static value_type FromText(const QString& string)
    {
        thread_local static QRegularExpression regExp(R"(\{\(([^\|]+)\)\|\(([^\}]*)\)\})");
        value_type result;

        auto it = regExp.globalMatch(string);
        while(it.hasNext()) {
            auto n = it.next();
            result.insert(TextConverter<Key>::FromText(n.captured(1)), TextConverter<Value>::FromText(n.captured(2)));
        }

        return result;
    }
};

template <typename Key>
struct TextConverter<QSet<Key>>
{
    using value_type = QSet<Key>;
    static QString ToText(const value_type& value, const TextConverterContext& context)
    {
        QString result;
        for(const auto& key : value) {
            result += "(" + TextConverter<Key>::ToText(key, context) + ")";
        }
        return result;
    }

    static value_type FromText(const QString& string)
    {
        thread_local static QRegularExpression regExp(R"(\(([^\)]+)\))");
        value_type result;

        auto it = regExp.globalMatch(string);
        while(it.hasNext()) {
            result.insert(TextConverter<Key>::FromText(it.next().captured(1)));
        }
        return result;
    }
};

template<>
struct TextConverter<QPoint>
{
    using value_type = QPoint;
    static QString ToText(const value_type& value, const TextConverterContext& context)
    {
        return "(" + TextConverter<qint32>::ToText(value.x(), context) + "," + TextConverter<qint32>::ToText(value.y(), context) + ")";
    }

    static value_type FromText(const QString& string)
    {
        thread_local static QRegularExpression regExp(R"(\(([^,]+),([^\)]+)\))");
        value_type result;
        auto it = regExp.globalMatch(string);
        while(it.hasNext()) {
            auto n = it.next();
            result.setX(TextConverter<qint32>::FromText(n.captured(1)));
            result.setY(TextConverter<qint32>::FromText(n.captured(2)));
        }

        return result;
    }
};

#define DECLARE_TEXT_CONVERTER_ALIAS(SourceType, Type) \
template<> \
struct TextConverter<Type> \
{ \
    using value_type = Type; \
    static QString ToText(const value_type& value, const TextConverterContext& context) \
    { \
        return TextConverter<SourceType>::ToText((SourceType)value, context); \
    } \
\
    static value_type FromText(const QString& string) \
    { \
        return (value_type)TextConverter<SourceType>::FromText(string); \
    } \
};

#endif // QSTRINGCONVERTERS_H
