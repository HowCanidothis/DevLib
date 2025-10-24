#include "thermalconductivitydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace ThermalConductivityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(WattMeterCelsius,             TR(MeasurementTr::tr("watt/meter/Celsius")),               TR(MeasurementTr::tr("W/(m*C)")),           1.0)
    IMPLEMENT_MEASUREMENT_UNIT(FootHourSquareFootFahrenheit, TR(MeasurementTr::tr("foot/hour/square foot/Fahrenheit")), TR(MeasurementTr::tr("BTU/(Hr*ft2*F)")),    1.7295772)

    IMPLEMENT_GLOBAL(double, Mud, WattMeterCelsius.FromUnitToBase(0.635));
    IMPLEMENT_GLOBAL(double, Water, WattMeterCelsius.FromUnitToBase(0.6));
    IMPLEMENT_GLOBAL(double, Steel, WattMeterCelsius.FromUnitToBase(40.0));
    IMPLEMENT_GLOBAL(double, Riser, WattMeterCelsius.FromUnitToBase(15.49));
    IMPLEMENT_GLOBAL(double, Cement, WattMeterCelsius.FromUnitToBase(0.7));
}

IMPLEMENT_MEASUREMENT(ThermalConductivity, TR(MeasurementTr::tr("Thermal Conductivity")))
