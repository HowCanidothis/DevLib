#ifndef FORCEDECLARATIONS_H
#define FORCEDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace ForceUnits
{
    DECLARE_MEASUREMENT_UNIT(Newton);
    DECLARE_MEASUREMENT_UNIT(Kilonewton);
    DECLARE_MEASUREMENT_UNIT(PoundsForce);
    DECLARE_MEASUREMENT_UNIT(KiloGrammForce);
    DECLARE_MEASUREMENT_UNIT(KiloPoundsForce);
};

DECLARE_MEASUREMENT(Force)

#endif // FORCEDECLARATIONS_H
