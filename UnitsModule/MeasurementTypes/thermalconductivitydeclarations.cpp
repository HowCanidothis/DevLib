#include "thermalconductivitydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace ThermalConductivityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(WattMeterCelsius, []{return QObject::tr("watt/meter/Celsius");}, []{ return QObject::tr("W/(m*С)"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(FootHourSquareFootFahrenheit, []{return QObject::tr("foot/hour/square foot/Fahrenheit");}, []{ return QObject::tr("BTU/(Hr*ft2*F)"); }, 1.7295772);
};

IMPLEMENT_MEASUREMENT(ThermalConductivity)
