#ifndef THERMALCONDUCTIVITYDECLARATIONS_H
#define THERMALCONDUCTIVITYDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace ThermalConductivityUnits
{
    DECLARE_MEASUREMENT_UNIT(WattMeterCelsius)
    DECLARE_MEASUREMENT_UNIT(FootHourSquareFootFahrenheit)

    DECLARE_GLOBAL(double, Mud)
    DECLARE_GLOBAL(double, Water)
    DECLARE_GLOBAL(double, Steel)
    DECLARE_GLOBAL(double, Riser)
    DECLARE_GLOBAL(double, Cement)
};

DECLARE_MEASUREMENT(ThermalConductivity)

#endif // TERMALCONDUCTIVITYDECLARATIONS_H
