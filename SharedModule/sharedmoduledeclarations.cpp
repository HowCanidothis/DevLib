#include "declarations.h"
#include <QDebug>
#include <QDir>

Q_LOGGING_CATEGORY(LC_UI, "ui")
Q_LOGGING_CATEGORY(LC_SYSTEM, "system")

double Nanosecs::TimesPerSecond() const
{
    return 1000000000.0 / _nsecs;
}

QString Nanosecs::ToString(const QString& caption) const
{
    return Timer::TimeToStringSecs(caption, _nsecs);
}

IMPLEMENT_GLOBAL(FTranslationHandler, TR_NONE, []{ return QString(); })

IMPLEMENT_DEFAULT(QString);
