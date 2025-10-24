#ifndef DENSITYDECLARATIONS_H
#define DENSITYDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace DensityUnits
{
    DECLARE_MEASUREMENT_UNIT(KilogramsPerCubicMeters)
    DECLARE_MEASUREMENT_UNIT(KilogramsPerLiter)
    DECLARE_MEASUREMENT_UNIT(PoundsPerGallon)
    DECLARE_MEASUREMENT_UNIT(PoundsPerCubicFeet)
    DECLARE_MEASUREMENT_UNIT(MilligrammPerLiter)
    DECLARE_MEASUREMENT_UNIT(SpecificGravity)

    DECLARE_GLOBAL(double, Precision)

    DECLARE_GLOBAL(double, Mud)
    DECLARE_GLOBAL(double, Oil)
    DECLARE_GLOBAL(double, Pore)
    DECLARE_GLOBAL(double, Water)
    DECLARE_GLOBAL(double, SeaWater)
    DECLARE_GLOBAL(double, Steel)
    DECLARE_GLOBAL(double, Riser)
    DECLARE_GLOBAL(double, Cement)
    DECLARE_GLOBAL(double, Cutting)

    DECLARE_GLOBAL(double, Min)
    DECLARE_GLOBAL(double, Max)
};

DECLARE_MEASUREMENT(Density)
DECLARE_MEASUREMENT(SolidDensity)
DECLARE_MEASUREMENT(MudWeight)

#endif // DENSITYDECLARATIONS_H
