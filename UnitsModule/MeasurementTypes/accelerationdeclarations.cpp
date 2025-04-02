#include "accelerationdeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace AccelerationUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSqSec,      TR(MeasurementTr::tr("Meters Per Square Second")),    TR(MeasurementTr::tr("m/s2")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSqMinute,   TR(MeasurementTr::tr("Meters Per Square Minute")),    TR(MeasurementTr::tr("m/min2")),   3600.0)
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSqHour,     TR(MeasurementTr::tr("Meters Per Square Hour")),      TR(MeasurementTr::tr("m/h2")),     3600.0 * 3600.)
    IMPLEMENT_MEASUREMENT_UNIT(KilometersPerSqHour, TR(MeasurementTr::tr("Kilometers Per Square Hour")),  TR(MeasurementTr::tr("km/h2")),    3600.0 * 3600. / 1000)
    IMPLEMENT_MEASUREMENT_UNIT(FeetsPerSqSec,       TR(MeasurementTr::tr("Feets Per Square Second")),     TR(MeasurementTr::tr("ft/s2")),    1./ METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(FeetsPerSqMinute,    TR(MeasurementTr::tr("Feets Per Square Minute")),     TR(MeasurementTr::tr("ft/min2")),  3600./ METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(FeetsPerSqHour,      TR(MeasurementTr::tr("Feets Per Square Hour")),       TR(MeasurementTr::tr("ft/h2")),    3600. * 3600. / METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(USFeetsPerSqSec,     TR(MeasurementTr::tr("USFeets Per Square Second")),   TR(MeasurementTr::tr("usft/s2")),   1. / METERS_TO_USFEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(USFeetsPerSqMinute,  TR(MeasurementTr::tr("USFeets Per Square Minute")),   TR(MeasurementTr::tr("usft/m2")),   3600. / METERS_TO_USFEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(USFeetsPerSqHour,    TR(MeasurementTr::tr("USFeets Per Square Hour")),     TR(MeasurementTr::tr("usft/h2")),   3600. * 3600. / METERS_TO_USFEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(Gravity,             TR(MeasurementTr::tr("Gravity")),                     TR(MeasurementTr::tr("g")),         9.80665)
}

IMPLEMENT_MEASUREMENT(Acceleration, TR(MeasurementTr::tr("Acceleration")))
