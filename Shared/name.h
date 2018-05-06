#ifndef NAME_H
#define NAME_H

#include <QString>

class Name
{
    qint32 value;
    QString transcription;
public:
    Name(const char* name);
    explicit Name(const QString& name);

    operator qint32() const { return value; }
    const QString& asString() const;
};

#endif // NAME_H
