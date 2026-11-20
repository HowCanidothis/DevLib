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
};

DECLARE_MEASUREMENT(Density)
DECLARE_MEASUREMENT(SolidDensity)
DECLARE_MEASUREMENT(MudWeight)

#endif // DENSITYDECLARATIONS_H
