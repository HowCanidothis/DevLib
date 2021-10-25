#ifndef TEMPERATUREDECLARATIONS_H
#define TEMPERATUREDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_TEMPERATURE = "Temperature";

namespace TemperatureUnits
{
    static const MeasurementUnit Celsius("Celsius", []{return QObject::tr("Celsius");}, []{ return QObject::tr("C"); }, [](double c)->double{return c * 1.8 + 32;}, [](double fa)->double{return ( fa - 32)/1.8;});
    static const MeasurementUnit Fahrenheit("Fahrenheit", []{return QObject::tr("Fahrenheit");}, []{ return QObject::tr("F"); }, 1);
};

#define MEASUREMENT_TEMPERATURE_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_TEMPERATURE)->FromUnitToBase(x)
#define MEASUREMENT_TEMPERATURE_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_TEMPERATURE)->FromBaseToUnit(x)
#define MEASUREMENT_TEMPERATURE_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_TEMPERATURE)->Precision
#define MEASUREMENT_TEMPERATURE_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_TEMPERATURE_BASE_TO_UNIT(x), 'f', MEASUREMENT_TEMPERATURE_PRECISION())

#define MEASUREMENT_TEMPERATURE_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_TEMPERATURE)->CurrentUnitLabel

#endif // TEMPERATUREDECLARATIONS_H
