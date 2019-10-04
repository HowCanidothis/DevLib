#ifndef NAME_H
#define NAME_H

#include <QString>
#include "shared_decl.h"

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

private:
    QString m_text;
    qint32 m_value;
};

class _Export Latin1Name
{
public:
    Latin1Name();
    Latin1Name(const char* name);
    Latin1Name(const QLatin1String& name);

    operator qint32() const { return m_value; }
    const QLatin1String& AsLatin1String() const { return m_text; }

    void SetName(const QLatin1String& str);

private:
    QLatin1String m_text;
    qint32 m_value;
};

#endif // NAME_H
