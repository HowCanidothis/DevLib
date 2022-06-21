#include "powerdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace PowerUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Watt, []{return QObject::tr("Watt");}, []{ return QObject::tr("W"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(JoulePerSecond, []{return QObject::tr("Joule Per Second");}, []{ return QObject::tr("J/s"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(KiloWatt, []{return QObject::tr("KiloWatt");}, []{ return QObject::tr("kW"); }, 1000.0);
    IMPLEMENT_MEASUREMENT_UNIT(HoursePower, []{return QObject::tr("Hourse Power");}, []{ return "hp"; }, 735.49875);
};

IMPLEMENT_MEASUREMENT(Power)
