#include "specificheatcapacitydeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace SpecificHeatCapacityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(JouleKilogramCelsius, []{return QObject::tr("joule/kilogram/Celsius");}, []{ return QObject::tr("J/(kg*C°)"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(PoundFahrenheit, []{return QObject::tr("pound/Fahrenheit");}, []{ return QObject::tr("BTU/(lb*F)"); }, 4184.0);
};

IMPLEMENT_MEASUREMENT(SpecificHeatCapacity, TR(MeasurementTr::tr("Specific Heat Capacity")))
