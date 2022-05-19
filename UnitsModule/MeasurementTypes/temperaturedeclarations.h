#ifndef TEMPERATUREDECLARATIONS_H
#define TEMPERATUREDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_TEMPERATURE = "Temperature";

namespace TemperatureUnits
{
    static const MeasurementUnit Celsius("Celsius", []{return QObject::tr("Celsius");}, []{ return QObject::tr("C°"); }, 1.0);
    static const MeasurementUnit Fahrenheit("Fahrenheit", []{return QObject::tr("Fahrenheit");}, []{ return QObject::tr("F"); }, [](double f)->double{return (f - 32.0) * 5.0 / 9.0;}, [](double c)->double{return (c * 9.0/5.0) + 32.0;});
};

static const Name MEASUREMENT_TEMPERATURE_PER_DISTANCE = "TemperaturePerDistance";

namespace TemperaturePerDistanceUnits
{
    static const MeasurementUnit CelsiusPerMeter        ("CelsiusPerMeter",         []{return QObject::tr("Celsius Per Meter");},       []{ return QObject::tr("C°/m"); }, 1.0);
    static const MeasurementUnit CelsiusPerFeet         ("CelsiusPerFeet",          []{return QObject::tr("Celsius Per Feet");},        []{ return QObject::tr("C°/ft"); }, METERS_TO_FEETS_MULTIPLIER);
    static const MeasurementUnit CelsiusPer100Feet      ("CelsiusPer100Feet",       []{return QObject::tr("Celsius Per 100 Feet");},    []{ return QObject::tr("C°/100ft"); }, METERS_TO_FEETS_MULTIPLIER/100);
    static const MeasurementUnit FahrenheitPerMeter     ("FahrenheitPerMeter",      []{return QObject::tr("Fahrenheit Per Meter");},    []{ return QObject::tr("F/m"); }, 5.0/9);
    static const MeasurementUnit FahrenheitPerFeet      ("FahrenheitPerFeet",       []{return QObject::tr("Fahrenheit Per Feet");},     []{ return QObject::tr("F/ft"); }, METERS_TO_FEETS_MULTIPLIER*5/9);
    static const MeasurementUnit FahrenheitPer100Feet   ("FahrenheitPer100Feet",    []{return QObject::tr("Fahrenheit Per 100 Feet");}, []{ return QObject::tr("F/100ft"); }, METERS_TO_FEETS_MULTIPLIER/100*5/9);
};

//namespace TemperatureInverseUnits
//{
//    static const MeasurementUnit InvCelsius("InvCelsius", []{return QObject::tr("Invert Celsius");}, []{ return QObject::tr("1 / C°"); }, 1.0);
//    static const MeasurementUnit InvFahrenheit("Fahrenheit", []{return QObject::tr("Invert Fahrenheit");}, []{ return QObject::tr("1 / F"); }, [](double f)->double{return (f - 32.0) * 5.0 / 9.0;}, [](double c)->double{return 1/ ((c * 9.0/5.0) + 32.0);});
//};


#define MEASUREMENT_TEMPERATURE_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_TEMPERATURE)->FromUnitToBase(x)
#define MEASUREMENT_TEMPERATURE_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_TEMPERATURE)->FromBaseToUnit(x)
#define MEASUREMENT_TEMPERATURE_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_TEMPERATURE)->CurrentPrecision
#define MEASUREMENT_TEMPERATURE_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_TEMPERATURE_BASE_TO_UNIT(x), 'f', MEASUREMENT_TEMPERATURE_PRECISION())

#define MEASUREMENT_TEMPERATURE_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_TEMPERATURE)->CurrentUnitLabel

#endif // TEMPERATUREDECLARATIONS_H
