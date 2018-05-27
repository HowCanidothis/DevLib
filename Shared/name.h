#ifndef NAME_H
#define NAME_H

#include <QString>

class Name
{
public:
    Name();
    Name(const char* name);
    Name(const Name& other);
    explicit Name(const QString& name);

    void SetName(const QString& str);

    operator qint32() const { return _value; }
    const QString& AsString() const;

private:
    QString _text;
    qint32 _value;
};

#endif // NAME_H
