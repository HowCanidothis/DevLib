#ifndef NAME_H
#define NAME_H

#include <QString>
#include "declarations.h"
#include "smartpointersadapters.h"

class _Export Name
{
public:
    Name();
    Name(const char* name);
    Name(const Name& other);
    explicit Name(const QString& name);

    void SetName(const QString& str);

    static Name Joined(const std::initializer_list<Name>& names);
    Name Joined(const Name& name) const { return Joined(name.AsString()); }
    Name Joined(const QString& string) const;
    Name Joined(const char* string) const { return Joined(QString(string)); }
    operator qint64() const { return m_value; }
    const QString& AsString() const;
    qint32 GetSize() const { return AsString().size(); }
    bool IsNull() const { return AsString().isEmpty(); }

    friend QDebug operator<<(QDebug debug, const Name& name) {
        debug << name.AsString() << " " << name.m_value;
        return debug.maybeSpace();
    }

    bool operator==(const Name& another) const { return another.m_value == m_value && (m_text.isEmpty() || another.m_text.isEmpty() || m_text == another.m_text); }

    static Name FromValue(qint64 value) { Name result; result.m_value = value; return result; }

private:
    QString m_text;
    qint64 m_value;
};

class _Export Latin1Name
{
public:
    Latin1Name();
    Latin1Name(const char* name);
    explicit Latin1Name(const std::string& name);
    explicit Latin1Name(std::string&& name);

    operator qint64() const { return m_value; }
    const std::string& AsLatin1String() const { return *m_text; }
    QString ToWideString() const { return QString::fromStdString(*m_text); }
    bool IsNull() const { return AsLatin1String().empty(); }

    bool operator==(const Latin1Name& another) const { return another.m_value == m_value && (m_text->empty() || another.m_text->empty() || *m_text == *another.m_text); }

    void SetName(const std::string& str);

    friend QDebug operator<<(QDebug debug, const Latin1Name& name) {
        debug.nospace();
        for(const auto& ch : name.AsLatin1String()) {
            debug << ch;
        }
        debug.space() << name.m_value;
        return debug.maybeSpace();
    }

private:
    SharedPointer<std::string> m_text;
    qint64 m_value;
};

struct UniName
{
    Name m_name;
    Latin1Name m_latinName;
public:
    UniName(const char* name);
    explicit UniName(const std::string& name);
    explicit UniName(std::string&& name);
    explicit UniName(const QString& name);
    explicit UniName(const Latin1Name& name);
    explicit UniName(const Name& name);

    void SetName(const std::string& name);
    void SetName(const QString& name);

    const Name& AsName() const { return m_name; }
    const Latin1Name& AsLatin1Name() const { return m_latinName; }
    const std::string& AsLatin1String() const { return m_latinName.AsLatin1String(); }
    const QString& AsString() const { return m_name.AsString(); }

    bool operator==(const Latin1Name& another) const { return another == m_name; }
    bool operator==(const Name& another) const { return another == m_latinName; }

    operator qint64() const { return m_name; }

    friend QDebug operator<<(QDebug debug, const UniName& name) {
        debug.nospace();
        for(const auto& ch : name.m_latinName.AsLatin1String()) {
            debug << ch;
        }
        debug.space() << name.m_latinName << name.m_name.AsString() << " " << (qint64)name.m_name;
        return debug.maybeSpace();
    }
};

template<class Enum>
class StringsMapper
{
public:
    StringsMapper& Register(const Name& name, Enum value)
    {
        m_strToEnum.insert(name, (qint64)value);
        m_enumToStr.insert((qint64)value, name);
        return *this;
    }

    const Name& GetStringOf(Enum value) const
    {
        static Name defaultResult;
        auto foundIt = m_enumToStr.find((qint64)value);
        if(foundIt != m_enumToStr.end()) {
            return foundIt.value();
        }
        return defaultResult;
    }

    Enum GetEnumOf(const QString& string) const
    {
        return GetEnumOf(Name(string.toLower()));
    }

    Enum GetEnumOf(const Name& name) const
    {
        auto foundIt = m_strToEnum.find(name);
        if(foundIt != m_strToEnum.end()) {
            return (Enum)foundIt.value();
        }
        return (Enum)-1;
    }

    template<class T>
    StringsMapper<T>& Converted() { return reinterpret_cast<StringsMapper<T>&>(*this); }

private:
    QHash<Name, qint64> m_strToEnum;
    QHash<qint64, Name> m_enumToStr;
};

Q_DECLARE_METATYPE(Name)

#define DECLARE_GLOBAL_NAME(x) \
    DECLARE_GLOBAL(Name,x)
#define IMPLEMENT_GLOBAL_NAME(x, value) \
    IMPLEMENT_GLOBAL(Name, x, QT_STRINGIFY(value));
#define IMPLEMENT_GLOBAL_NAME_1(x) \
    IMPLEMENT_GLOBAL(Name, x, QT_STRINGIFY(x));

#endif // NAME_H
