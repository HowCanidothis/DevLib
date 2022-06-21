#ifndef SPEEDDECLARATIONS_H
#define SPEEDDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

#define AnnularVelocityUnits SpeedUnits
namespace SpeedUnits
{
    DECLARE_MEASUREMENT_UNIT(MetersPerSecond)
    DECLARE_MEASUREMENT_UNIT(MetersPerMinute)
    DECLARE_MEASUREMENT_UNIT(MetersPerHour)
    DECLARE_MEASUREMENT_UNIT(KilometersPerHour)
    DECLARE_MEASUREMENT_UNIT(FeetPerHour)
    DECLARE_MEASUREMENT_UNIT(USfeetPerHour)
    DECLARE_MEASUREMENT_UNIT(FeetPerMinute)
    DECLARE_MEASUREMENT_UNIT(USfeetPerMinute)
    DECLARE_MEASUREMENT_UNIT(FeetPerSecond)
    DECLARE_MEASUREMENT_UNIT(USfeetPerSecond)
    DECLARE_MEASUREMENT_UNIT(MilesPerHour)
}

DECLARE_MEASUREMENT(Speed)
DECLARE_MEASUREMENT(AnnularVelocity)

#endif // SPEEDDECLARATIONS_H
