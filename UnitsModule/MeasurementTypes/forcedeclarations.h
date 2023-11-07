#ifndef FORCEDECLARATIONS_H
#define FORCEDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace ForceUnits
{
    DECLARE_MEASUREMENT_UNIT(Newton)
    DECLARE_MEASUREMENT_UNIT(Kilonewton)
    DECLARE_MEASUREMENT_UNIT(PoundsForce)
    DECLARE_MEASUREMENT_UNIT(KiloGrammForce)
    DECLARE_MEASUREMENT_UNIT(KiloPoundsForce)
}

DECLARE_MEASUREMENT(Force)

namespace NormalizedForceUnits
{
    DECLARE_MEASUREMENT_UNIT(NewtonLength)
    DECLARE_MEASUREMENT_UNIT(KilonewtonLength)
    DECLARE_MEASUREMENT_UNIT(PoundsForceLength)
    DECLARE_MEASUREMENT_UNIT(KiloGrammForceLength)
    DECLARE_MEASUREMENT_UNIT(KiloPoundsForceLength)
}

DECLARE_MEASUREMENT(NormalizedForce)
#endif // FORCEDECLARATIONS_H
