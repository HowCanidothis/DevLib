#ifndef TIMER_H
#define TIMER_H

#include <QString>

class QTextStream;

class Timer{
    typedef qint64 nsecs;
    typedef qint32 msecs;
    nsecs _time;
public:
    Timer();

    void Bind();
    nsecs Release();
    static msecs ToMsecs(nsecs nanosecs);

    static QString TimeToStringSecs(const QString& lbl, qint64 time);
    QString TimeToStringSecs(const QString& lbl) const { return TimeToStringSecs(lbl, _time); }

    static qint64 Now();
};

#endif // TIMER_H
