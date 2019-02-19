#ifndef QPROCESSBASE_H
#define QPROCESSBASE_H

#include <QString>

#include "processbase.h"

class _Export QProcessBase : public ProcessBase
{
    typedef ProcessBase Super;
public:
    QProcessBase(){}

    void BeginProcess(const QString& title, bool shadow = false);
    void BeginProcess(const QString& title, int stepsCount, bool shadow = false);
    void SetProcessTitle(const QString& title);
};

#endif // QPROCESSBASE_H
