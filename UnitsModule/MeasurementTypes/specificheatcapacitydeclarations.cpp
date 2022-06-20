#ifndef SPECIFICHEATCAPACITYDECLARATIONS_H
#define SPECIFICHEATCAPACITYDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_SPECIFIC_HEAT_CAPACITY = "SpecificHeatCapacity";

namespace SpecificHeatCapacityUnits
{
    static const MeasurementUnit JouleKilogramCelsius("JouleKilogramCelsius", []{return QObject::tr("joule/kilogram/Celsius");}, []{ return QObject::tr("J/(kg*C°)"); }, 1.0);
    static const MeasurementUnit PoundFahrenheit("PoundFahrenheit", []{return QObject::tr("pound/Fahrenheit");}, []{ return QObject::tr("BTU/(lb*F)"); }, 4184.0);
};

#define MEASUREMENT_SPECIFIC_HEAT_CAPACITY_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_SPECIFIC_HEAT_CAPACITY)->FromUnitToBase(x)
#define MEASUREMENT_SPECIFIC_HEAT_CAPACITY_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_SPECIFIC_HEAT_CAPACITY)->FromBaseToUnit(x)
#define MEASUREMENT_SPECIFIC_HEAT_CAPACITY_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_SPECIFIC_HEAT_CAPACITY)->CurrentPrecision
#define MEASUREMENT_SPECIFIC_HEAT_CAPACITY_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_SPECIFIC_HEAT_CAPACITY_BASE_TO_UNIT(x), 'f', MEASUREMENT_SPECIFIC_HEAT_CAPACITY_PRECISION())

#define MEASUREMENT_SPECIFIC_HEAT_CAPACITY_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_SPECIFIC_HEAT_CAPACITY)->CurrentUnitLabel
#define ATTACH_SPECIFIC_HEAT_CAPACITY_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_SPECIFIC_HEAT_CAPACITY, delegate, min, max)


#endif // SPECIFICHEATCAPACITYDECLARATIONS_H
