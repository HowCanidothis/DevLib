#include "timedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace TimeUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(MSeconds,   []{return QObject::tr("msec");}, []{ return QObject::tr("ms"); }, 0.001);
    IMPLEMENT_MEASUREMENT_UNIT(Seconds,    []{return QObject::tr("sec");}, []{ return QObject::tr("s"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Minutes,    []{return QObject::tr("min");}, []{ return QObject::tr("min"); }, 60.0);
    IMPLEMENT_MEASUREMENT_UNIT(Hours,      []{return QObject::tr("hour");}, []{ return QObject::tr("hr"); }, 3600.0);
    IMPLEMENT_MEASUREMENT_UNIT(Day,      []{return QObject::tr("days");}, []{ return QObject::tr("day"); }, 86400.0);
    IMPLEMENT_MEASUREMENT_UNIT(Months,      []{return QObject::tr("months");}, []{ return QObject::tr("month"); }, 2628000.0);
    IMPLEMENT_MEASUREMENT_UNIT(Years,      []{return QObject::tr("years");}, []{ return QObject::tr("year"); }, 31536000.0);
};

IMPLEMENT_MEASUREMENT(Time, TR(MeasurementTr::tr("Time")))
