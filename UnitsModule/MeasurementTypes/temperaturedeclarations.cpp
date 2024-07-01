#include "temperaturedeclarations.h"

#include "UnitsModule/measurementunitmanager.h"


namespace TemperatureUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Kelvin, TR(MeasurementTr::tr("Kelvin")), TR(MeasurementTr::tr("K")), 1.)
    IMPLEMENT_MEASUREMENT_UNIT(Celsius, TR(MeasurementTr::tr("Celsius")), TR(MeasurementTr::tr("°C")), [](double c) {return c + 273.15; }, [](double k){return k - 273.15; })
    IMPLEMENT_MEASUREMENT_UNIT(Fahrenheit, TR(MeasurementTr::tr("Fahrenheit")), TR(MeasurementTr::tr("°F")), [](double f){return (f - 32.0) * 5.0 / 9.0 + 273.15; }, [](double k){return (k - 273.15) * 9.0/5.0 + 32.0; })
}

namespace TemperaturePerDistanceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KelvinPerMeter,          TR(MeasurementTr::tr("Kelvin Per Meter")),        TR(MeasurementTr::tr("K/m")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(CelsiusPerMeter,         TR(MeasurementTr::tr("Celsius Per Meter")),       TR(MeasurementTr::tr("C°/m")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(CelsiusPerFeet,          TR(MeasurementTr::tr("Celsius Per Feet")),        TR(MeasurementTr::tr("C°/ft")),   METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(CelsiusPer100Feet,       TR(MeasurementTr::tr("Celsius Per 100 Feet")),    TR(MeasurementTr::tr("C°/100ft")),METERS_TO_FEETS_MULTIPLIER / 100.0)
    IMPLEMENT_MEASUREMENT_UNIT(FahrenheitPerMeter,      TR(MeasurementTr::tr("Fahrenheit Per Meter")),    TR(MeasurementTr::tr("F/m")),     5.0 / 9.0)
    IMPLEMENT_MEASUREMENT_UNIT(FahrenheitPerFeet,       TR(MeasurementTr::tr("Fahrenheit Per Feet")),     TR(MeasurementTr::tr("F/ft")),    METERS_TO_FEETS_MULTIPLIER * 5.0 / 9.0)
    IMPLEMENT_MEASUREMENT_UNIT(FahrenheitPer100Feet,    TR(MeasurementTr::tr("Fahrenheit Per 100 Feet")), TR(MeasurementTr::tr("F/100ft")), METERS_TO_FEETS_MULTIPLIER / 100.0 * 5.0 / 9.0)
}

IMPLEMENT_MEASUREMENT(Temperature, TR(MeasurementTr::tr("Temperature")))
IMPLEMENT_MEASUREMENT(TemperaturePerDistance, TR(MeasurementTr::tr("Temperature Per Distance")))
