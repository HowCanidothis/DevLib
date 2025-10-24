#ifndef SPECIFICHEATCAPACITYDECLARATIONS_H
#define SPECIFICHEATCAPACITYDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace SpecificHeatCapacityUnits
{
    DECLARE_MEASUREMENT_UNIT(JouleKilogramCelsius)
    DECLARE_MEASUREMENT_UNIT(JouleKilogramKelvin)
    DECLARE_MEASUREMENT_UNIT(PoundFahrenheit)

    DECLARE_GLOBAL(double, Mud)
    DECLARE_GLOBAL(double, Water)
    DECLARE_GLOBAL(double, Steel)
    DECLARE_GLOBAL(double, Riser)
    DECLARE_GLOBAL(double, Cement)
}

DECLARE_MEASUREMENT(SpecificHeatCapacity)

#endif // SPECIFICHEATCAPACITYDECLARATIONS_H
