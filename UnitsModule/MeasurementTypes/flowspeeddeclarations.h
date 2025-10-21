#ifndef FLOWSPEEDDECLARATIONS_H
#define FLOWSPEEDDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace FlowSpeedUnits
{
    DECLARE_MEASUREMENT_UNIT(CubicMetersPerSecond)
    DECLARE_MEASUREMENT_UNIT(CubicMetersPerMinute)
    DECLARE_MEASUREMENT_UNIT(CubicMetersPerHour)
    DECLARE_MEASUREMENT_UNIT(CubicMetersPerDay)
    DECLARE_MEASUREMENT_UNIT(LitersPerSecond)
    DECLARE_MEASUREMENT_UNIT(LitersPerMinute)
    DECLARE_MEASUREMENT_UNIT(GallonsPerMinute)
    DECLARE_MEASUREMENT_UNIT(BarrelsPerMinute)
    DECLARE_MEASUREMENT_UNIT(CubicFeetPerSecond)
    DECLARE_MEASUREMENT_UNIT(MilliliterPer30Min)

    DECLARE_GLOBAL(double, Precision)
}

DECLARE_MEASUREMENT(FlowSpeed)
DECLARE_MEASUREMENT(Filtrate)


#endif // FLOWSPEEDDECLARATIONS_H
