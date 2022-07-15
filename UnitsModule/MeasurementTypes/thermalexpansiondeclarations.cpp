#include "thermalexpansiondeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace ThermalExpansionUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(PerDegreeCelsius, []{return QObject::tr("per degree Celsius");}, []{ return QObject::tr("E-06/C°"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(PerDegreeKelvin, []{return QObject::tr("per degree Kelvin");}, []{ return QObject::tr("E-06/K"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(PerDegreeFahrenheit, []{return QObject::tr("per degree Fahrenheit");}, []{ return QObject::tr("E-06/F"); }, 0.555555556);
};

IMPLEMENT_MEASUREMENT(ThermalExpansion)
