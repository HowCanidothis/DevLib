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

    ~Name();

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

    Name& operator=(const Name& another);
    bool operator==(const Name& another) const { return another.m_value == m_value && (m_text.isEmpty() || another.m_text.isEmpty() || m_text == another.m_text); }
    bool operator!=(const Name& another) const { return !operator==(another); }
    bool operator<(const Name& another) const { return m_text < another.m_text; }
    bool operator<=(const Name& another) const { return operator<(another) || operator==(another); }

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

class UniName
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

template<class T>
class StringsMapper
{
public:
    StringsMapper& RegisterCaseSensitive(const QString& name, const T& value)
    {
        m_strToEnum.insert(Name(name.toLower()), value);
        m_enumToStr.insert(value, Name(name));
        return *this;
    }

    StringsMapper& Register(const Name& name, const T& value)
    {
        m_strToEnum.insert(name, value);
        m_enumToStr.insert(value, name);
        return *this;
    }

    const Name& GetKeyOf(const T& value) const
    {
        static Name defaultResult;
        auto foundIt = m_enumToStr.find(value);
        if(foundIt != m_enumToStr.end()) {
            return foundIt.value();
        }
        return defaultResult;
    }

    const T& GetValueOf(const QString& string, const T& defValue) const
    {
        return GetValueOf(Name(string.toLower()), defValue);
    }

    const T& GetValueOf(const Name& name, const T& defValue) const
    {
        auto foundIt = m_strToEnum.find(name);
        if(foundIt != m_strToEnum.end()) {
            return foundIt.value();
        }
        return defValue;
    }

private:
    QHash<Name, T> m_strToEnum;
    QHash<T, Name> m_enumToStr;
};

template<class Enum>
class StringsToEnumMapper : public StringsMapper<qint64>
{
    using Super = StringsMapper<qint64>;
public:
    StringsToEnumMapper& RegisterCaseSensitive(const QString& name, Enum value)
    {
        Super::RegisterCaseSensitive(name, (qint64)value);
        return *this;
    }

    StringsToEnumMapper& Register(const Name& name, Enum value)
    {
        Super::Register(name, (qint64)value);
        return *this;
    }

    const Name& GetStringOf(Enum value) const
    {
        return Super::GetKeyOf((qint64)value);
    }

    Enum GetEnumOf(const QString& string) const
    {
        return GetEnumOf(Name(string.toLower()));
    }

    Enum GetEnumOf(const Name& name) const
    {
        return (Enum)GetValueOf(name, -1);
    }

    template<class T>
    StringsToEnumMapper<T>& Converted() { return reinterpret_cast<StringsToEnumMapper<T>&>(*this); }
};

#ifdef QT_DEBUG
class DebugNameManager : public Singletone<DebugNameManager>
{
public:
    DebugNameManager();
    void PrintReport(qint32 maxSymbolUsage = 64);

private:
    friend class Name;
    void addName(const QString& name);
    void removeName(const QString& name);

    QHash<QString, QHash<const QChar*, qint32>> m_counter;
    QMutex m_mutex;
};

#endif

Q_DECLARE_METATYPE(Name)

#define DECLARE_GLOBAL_NAME(x) \
    DECLARE_GLOBAL(Name,x)
#define IMPLEMENT_GLOBAL_NAME(x, value) \
    IMPLEMENT_GLOBAL(Name, x, QT_STRINGIFY(value));
#define IMPLEMENT_GLOBAL_NAME_1(x) \
    IMPLEMENT_GLOBAL(Name, x, QT_STRINGIFY(x));

#endif // NAME_H
