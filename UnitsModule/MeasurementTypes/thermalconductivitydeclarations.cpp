#include "thermalconductivitydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace ThermalConductivityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(WattMeterCelsius,             TR(MeasurementTr::tr("watt/meter/Celsius")),               TR(MeasurementTr::tr("W/(m*C)")),           1.0)
    IMPLEMENT_MEASUREMENT_UNIT(FootHourSquareFootFahrenheit, TR(MeasurementTr::tr("foot/hour/square foot/Fahrenheit")), TR(MeasurementTr::tr("BTU/(Hr*ft2*F)")),    1.7295772)
}

IMPLEMENT_MEASUREMENT(ThermalConductivity, TR(MeasurementTr::tr("Thermal Conductivity")))
