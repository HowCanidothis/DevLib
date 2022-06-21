#ifndef TEMPERATUREDECLARATIONS_H
#define TEMPERATUREDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace TemperatureUnits
{
    DECLARE_MEASUREMENT_UNIT(Celsius)
    DECLARE_MEASUREMENT_UNIT(Fahrenheit)
};

namespace TemperaturePerDistanceUnits
{
    DECLARE_MEASUREMENT_UNIT(CelsiusPerMeter)
    DECLARE_MEASUREMENT_UNIT(CelsiusPerFeet)
    DECLARE_MEASUREMENT_UNIT(CelsiusPer100Feet)
    DECLARE_MEASUREMENT_UNIT(FahrenheitPerMeter)
    DECLARE_MEASUREMENT_UNIT(FahrenheitPerFeet)
    DECLARE_MEASUREMENT_UNIT(FahrenheitPer100Feet)
};

DECLARE_MEASUREMENT(Temperature)
DECLARE_MEASUREMENT(TemperaturePerDistance)

#endif // TEMPERATUREDECLARATIONS_H
