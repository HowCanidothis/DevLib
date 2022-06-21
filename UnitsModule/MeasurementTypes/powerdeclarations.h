#ifndef POWERDECLARATIONS_H
#define POWERDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace PowerUnits
{
    DECLARE_MEASUREMENT_UNIT(Watt)
    DECLARE_MEASUREMENT_UNIT(JoulePerSecond)
    DECLARE_MEASUREMENT_UNIT(KiloWatt)
    DECLARE_MEASUREMENT_UNIT(HoursePower)
};

DECLARE_MEASUREMENT(Power)

#endif // POWERDECLARATIONS_H
