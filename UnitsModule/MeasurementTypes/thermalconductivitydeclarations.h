#ifndef THERMALCONDUCTIVITYDECLARATIONS_H
#define THERMALCONDUCTIVITYDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_THERMAL_CONDUCTIVITY = "Thermal Conductivity";

namespace ThermalConductivityUnits
{
    static const MeasurementUnit WattMeterCelsius("WattMeterCelsius", []{return QObject::tr("watt/meter/Celsius");}, []{ return QObject::tr("W/(m*С)"); }, 1.0);
    static const MeasurementUnit FootHourSquareFootFahrenheit("FootHourSquareFootFahrenheit", []{return QObject::tr("foot/hour/square foot/Fahrenheit");}, []{ return QObject::tr("BTU/(Hr*ft2*F)"); }, 1.7295772);
};

#define MEASUREMENT_THERMAL_CONDUCTIVITY_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_THERMAL_CONDUCTIVITY)->FromUnitToBase(x)
#define MEASUREMENT_THERMAL_CONDUCTIVITY_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_THERMAL_CONDUCTIVITY)->FromBaseToUnit(x)
#define MEASUREMENT_THERMAL_CONDUCTIVITY_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_THERMAL_CONDUCTIVITY)->CurrentPrecision
#define MEASUREMENT_THERMAL_CONDUCTIVITY_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_THERMAL_CONDUCTIVITY_BASE_TO_UNIT(x), 'f', MEASUREMENT_THERMAL_CONDUCTIVITY_PRECISION())

#define MEASUREMENT_THERMAL_CONDUCTIVITY_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_THERMAL_CONDUCTIVITY)->CurrentUnitLabel
#define ATTACH_THERMAL_CONDUCTIVITY_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_THERMAL_CONDUCTIVITY, delegate, min, max)

#endif // TERMALCONDUCTIVITYDECLARATIONS_H
