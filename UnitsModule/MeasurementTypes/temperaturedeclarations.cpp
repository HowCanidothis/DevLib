#include "temperaturedeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace TemperatureUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Celsius, []{return QObject::tr("Celsius");}, []{ return QObject::tr("C°"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Fahrenheit, []{return QObject::tr("Fahrenheit");}, []{ return QObject::tr("F"); }, [](double f)->double{return (f - 32.0) * 5.0 / 9.0;}, [](double c)->double{return (c * 9.0/5.0) + 32.0;});
};

namespace TemperaturePerDistanceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CelsiusPerMeter,         []{return QObject::tr("Celsius Per Meter");},       []{ return QObject::tr("C°/m"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(CelsiusPerFeet,          []{return QObject::tr("Celsius Per Feet");},        []{ return QObject::tr("C°/ft"); }, METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(CelsiusPer100Feet,       []{return QObject::tr("Celsius Per 100 Feet");},    []{ return QObject::tr("C°/100ft"); }, METERS_TO_FEETS_MULTIPLIER/100);
    IMPLEMENT_MEASUREMENT_UNIT(FahrenheitPerMeter,      []{return QObject::tr("Fahrenheit Per Meter");},    []{ return QObject::tr("F/m"); }, 5.0/9);
    IMPLEMENT_MEASUREMENT_UNIT(FahrenheitPerFeet,       []{return QObject::tr("Fahrenheit Per Feet");},     []{ return QObject::tr("F/ft"); }, METERS_TO_FEETS_MULTIPLIER*5/9);
    IMPLEMENT_MEASUREMENT_UNIT(FahrenheitPer100Feet,    []{return QObject::tr("Fahrenheit Per 100 Feet");}, []{ return QObject::tr("F/100ft"); }, METERS_TO_FEETS_MULTIPLIER/100*5/9);
};

IMPLEMENT_MEASUREMENT(Temperature)
IMPLEMENT_MEASUREMENT(TemperaturePerDistance)
