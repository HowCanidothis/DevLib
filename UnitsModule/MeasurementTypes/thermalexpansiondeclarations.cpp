#include "thermalexpansiondeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace ThermalExpansionUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(PerDegreeCelsius,    TR(MeasurementTr::tr("per degree Celsius")),    TR(MeasurementTr::tr("E-06/C°")),   1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PerDegreeKelvin,     TR(MeasurementTr::tr("per degree Kelvin")),     TR(MeasurementTr::tr("E-06/K")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PerDegreeFahrenheit, TR(MeasurementTr::tr("per degree Fahrenheit")), TR(MeasurementTr::tr("E-06/F")),    1.8)
}

IMPLEMENT_MEASUREMENT(ThermalExpansion, TR(MeasurementTr::tr("Thermal Expansion")))
