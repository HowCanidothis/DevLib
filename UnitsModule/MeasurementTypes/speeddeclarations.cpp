#include "speeddeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace SpeedUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSecond,   TR(MeasurementTr::tr("Meters Per Second")),   TR(MeasurementTr::tr("m/s")),       1.0)
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerMinute,   TR(MeasurementTr::tr("Meters Per Minute")),   TR(MeasurementTr::tr("m/min")),     1.0 / 60.0 )
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerHour,     TR(MeasurementTr::tr("Meters Per Hour")),     TR(MeasurementTr::tr("m/hr")),      1.0 / 60.0 / 60.0 )
    IMPLEMENT_MEASUREMENT_UNIT(KilometersPerHour, TR(MeasurementTr::tr("Kilometers Per Hour")), TR(MeasurementTr::tr("km/hr")),     1000.0 / 60.0 / 60.0)
    IMPLEMENT_MEASUREMENT_UNIT(FeetPerHour,       TR(MeasurementTr::tr("Feet Per Hour")),       TR(MeasurementTr::tr("ft/hr")),     1.0 / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0 )
    IMPLEMENT_MEASUREMENT_UNIT(USfeetPerHour,     TR(MeasurementTr::tr("US Feet Per Hour")),    TR(MeasurementTr::tr("usft/hr")),   USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0 )
    IMPLEMENT_MEASUREMENT_UNIT(FeetPerMinute,     TR(MeasurementTr::tr("Feet Per Minute")),     TR(MeasurementTr::tr("ft/min")),    1.0 / 60.0 / METERS_TO_FEETS_MULTIPLIER )
    IMPLEMENT_MEASUREMENT_UNIT(USfeetPerMinute,   TR(MeasurementTr::tr("US Feet Per Minute")),  TR(MeasurementTr::tr("usft/min")),  USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER / 60.0)
    IMPLEMENT_MEASUREMENT_UNIT(FeetPerSecond,     TR(MeasurementTr::tr("Feet Per Second")),     TR(MeasurementTr::tr("ft/sec")),    1.0/ METERS_TO_FEETS_MULTIPLIER )
    IMPLEMENT_MEASUREMENT_UNIT(USfeetPerSecond,   TR(MeasurementTr::tr("US Feet Per Second")),  TR(MeasurementTr::tr("usft/sec")),  USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER )
    IMPLEMENT_MEASUREMENT_UNIT(MilesPerHour,      TR(MeasurementTr::tr("Miles Per Hour")),      TR(MeasurementTr::tr("mphr")),      5280 / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0)
}

IMPLEMENT_MEASUREMENT(Speed, TR(MeasurementTr::tr("Speed")))
IMPLEMENT_MEASUREMENT(AnnularVelocity, TR(MeasurementTr::tr("Annular Velocity")))
IMPLEMENT_MEASUREMENT(ROP, TR(MeasurementTr::tr("Rate Of Penetration")))
