#include "qprocessbase.h"

void QProcessBase::BeginProcess(const QString& title, bool shadow)
{
    Super::BeginProcess(title.toStdWString().c_str(), shadow);
}

void QProcessBase::BeginProcess(const QString& title, int stepsCount, bool shadow)
{
    Super::BeginProcess(title.toStdWString().c_str(), stepsCount, shadow);
}

void QProcessBase::SetProcessTitle(const QString& title)
{
    Super::SetProcessTitle(title.toStdWString().c_str());
}
