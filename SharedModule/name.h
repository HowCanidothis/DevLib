#ifndef NAME_H
#define NAME_H

#include <QString>
#include "shared_decl.h"
#include "smartpointersadapters.h"

class _Export Name
{
public:
    Name();
    Name(const char* name);
    Name(const Name& other);
    explicit Name(const QString& name);

    void SetName(const QString& str);

    operator qint32() const { return m_value; }
    const QString& AsString() const;
    bool IsNull() const { return AsString().isEmpty(); }

    friend QDebug operator<<(QDebug debug, const Name& name) {
        debug << name.AsString() << " " << name.m_value;
        return debug.maybeSpace();
    }

    bool operator==(const Name& another) const { return another.m_value == m_value && (m_text.isEmpty() || another.m_text.isEmpty() || m_text == another.m_text); }

    static Name FromValue(qint32 value) { Name result; result.m_value = value; return result; }

private:
    QString m_text;
    qint32 m_value;
};

class _Export Latin1Name
{
public:
    Latin1Name();
    Latin1Name(const char* name);
    Latin1Name(const std::string& name);

    operator qint32() const { return m_value; }
    const std::string& AsLatin1String() const { return *m_text; }
    bool IsNull() const { return AsLatin1String().empty(); }

    void SetName(const std::string& str);

private:
    SharedPointer<std::string> m_text;
    qint32 m_value;
};

#endif // NAME_H
