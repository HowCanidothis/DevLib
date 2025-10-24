#include "specificheatcapacitydeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace SpecificHeatCapacityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(JouleKilogramCelsius, TR(MeasurementTr::tr("joule/kilogram/Celsius")),   TR(MeasurementTr::tr("J/(kg*C°)")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(JouleKilogramKelvin,  TR(MeasurementTr::tr("joule/kilogram/Kelvin")),    TR(MeasurementTr::tr("J/(kg*K)")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundFahrenheit,      TR(MeasurementTr::tr("pound/Fahrenheit")),         TR(MeasurementTr::tr("BTU/(lb*F)")), 4184.0)

    IMPLEMENT_GLOBAL(double, Mud, JouleKilogramCelsius.FromUnitToBase(3713.0));
    IMPLEMENT_GLOBAL(double, Water, JouleKilogramCelsius.FromUnitToBase(4000.0));
    IMPLEMENT_GLOBAL(double, Steel, JouleKilogramCelsius.FromUnitToBase(464.0));
    IMPLEMENT_GLOBAL(double, Riser, JouleKilogramCelsius.FromUnitToBase(400.0));
    IMPLEMENT_GLOBAL(double, Cement, JouleKilogramCelsius.FromUnitToBase(2000.0));
}

IMPLEMENT_MEASUREMENT(SpecificHeatCapacity, TR(MeasurementTr::tr("Specific Heat Capacity")))
