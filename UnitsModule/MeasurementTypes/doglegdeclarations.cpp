#include "doglegdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace DLSUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Rad100Feet,      TR(MeasurementTr::tr("rad per 100ft")),     TR(MeasurementTr::tr("rad/100ft")), METERS_TO_FEETS_MULTIPLIER / 100.0)
    IMPLEMENT_MEASUREMENT_UNIT(RadMeter,        TR(MeasurementTr::tr("rad per m")),         TR(MeasurementTr::tr("rad/m")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Rad30Meter,      TR(MeasurementTr::tr("rad per 30m")),       TR(MeasurementTr::tr("rad/30m")),   1.0/30)
    IMPLEMENT_MEASUREMENT_UNIT(Degree100Feet,   TR(MeasurementTr::tr("deg per 100ft")),     TR(MeasurementTr::tr("°/100ft")),   DEGREES_TO_RADIANS * METERS_TO_FEETS_MULTIPLIER / 100.0)
    IMPLEMENT_MEASUREMENT_UNIT(Degree100USFeet, TR(MeasurementTr::tr("deg per 100usft")),   TR(MeasurementTr::tr("°/100usft")), DEGREES_TO_RADIANS * METERS_TO_FEETS_MULTIPLIER / (100.0 * USFEETS_TO_FEETS_MULTIPLIER))
    IMPLEMENT_MEASUREMENT_UNIT(Degree30Meter,   TR(MeasurementTr::tr("deg per 30m")),       TR(MeasurementTr::tr("°/30m")),     DEGREES_TO_RADIANS/30)
}

IMPLEMENT_MEASUREMENT(DLS, TR(MeasurementTr::tr("Dogleg Severity")))
