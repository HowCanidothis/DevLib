#include "qprocessbase.h"

void QProcessBase::BeginProcess(const QString& title)
{
    Super::BeginProcess(title.toStdWString().c_str(), false);
}

void QProcessBase::BeginShadowProcess(const QString& title)
{
    Super::BeginProcess(title.toStdWString().c_str(), true);
}

void QProcessBase::BeginProcess(const QString& title, int stepsCount, int wantedCount)
{
    Super::BeginProcess(title.toStdWString().c_str(), stepsCount, wantedCount, false);
}

void QProcessBase::BeginShadowProcess(const QString& title, int stepsCount, int wantedCount)
{
    Super::BeginProcess(title.toStdWString().c_str(), stepsCount, wantedCount, true);
}

void QProcessBase::SetProcessTitle(const QString& title)
{
    Super::SetProcessTitle(title.toStdWString().c_str());
}
