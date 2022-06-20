#include "doglegdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace DLSUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(RadFeet, []{return QObject::tr("rad per 100ft");}, []{ return QObject::tr("rad/100ft"); }, (30.0 * METERS_TO_FEETS_MULTIPLIER) / 100.0);
    IMPLEMENT_MEASUREMENT_UNIT(RadMeter, []{return QObject::tr("rad per 30m");}, []{ return QObject::tr("rad/30m"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(DegreeFeet, []{return QObject::tr("deg per 100ft");}, []{ return QObject::tr("°/100ft"); }, DEGREES_TO_RADIANS * (30.0 * METERS_TO_FEETS_MULTIPLIER) / 100.0);
    IMPLEMENT_MEASUREMENT_UNIT(DegreeUSFeet, []{return QObject::tr("deg per 100usft");}, []{ return QObject::tr("°/100usft"); }, DEGREES_TO_RADIANS * (30.0 * METERS_TO_FEETS_MULTIPLIER) / 100.0 / USFEETS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(DegreeMeter, []{return QObject::tr("deg per 30m");}, []{ return QObject::tr("°/30m"); }, DEGREES_TO_RADIANS);
}

IMPLEMENT_MEASUREMENT(MeasurementDLS)
