#ifndef QPROCESSBASE_H
#define QPROCESSBASE_H

#include <QString>

#include "processbase.h"

class _Export QProcessBase : public ProcessBase
{
    typedef ProcessBase Super;
public:
    QProcessBase(){}

    void BeginProcess(const QString& title);
    void BeginShadowProcess(const QString& title);
    void BeginProcess(const QString& title, int stepsCount, int wantedCount = 100);
    void BeginShadowProcess(const QString& title, int stepsCount, int wantedCount = 100);
    void SetProcessTitle(const QString& title);
};

#endif // QPROCESSBASE_H
