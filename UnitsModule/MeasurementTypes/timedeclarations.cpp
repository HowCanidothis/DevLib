#include "timedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace TimeUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(MSeconds,   TR(MeasurementTr::tr("msec")),   TR(MeasurementTr::tr("ms")),    0.001)
    IMPLEMENT_MEASUREMENT_UNIT(Seconds,    TR(MeasurementTr::tr("sec")),    TR(MeasurementTr::tr("s")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Minutes,    TR(MeasurementTr::tr("min")),    TR(MeasurementTr::tr("min")),   60.0)
    IMPLEMENT_MEASUREMENT_UNIT(Hours,      TR(MeasurementTr::tr("hour")),   TR(MeasurementTr::tr("hr")),    3600.0)
    IMPLEMENT_MEASUREMENT_UNIT(Day,        TR(MeasurementTr::tr("days")),   TR(MeasurementTr::tr("day")),   86400.0)
    IMPLEMENT_MEASUREMENT_UNIT(Months,     TR(MeasurementTr::tr("months")), TR(MeasurementTr::tr("month")), 2628000.0)
    IMPLEMENT_MEASUREMENT_UNIT(Years,      TR(MeasurementTr::tr("years")),  TR(MeasurementTr::tr("year")),  31536000.0)
}

IMPLEMENT_MEASUREMENT(Time, TR(MeasurementTr::tr("Time")))
IMPLEMENT_MEASUREMENT(SSTime, TR(MeasurementTr::tr("Surge&Swab Time")))
