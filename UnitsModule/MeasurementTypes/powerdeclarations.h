#ifndef POWERDECLARATIONS_H
#define POWERDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_POWER = "Power";

namespace PowerUnits
{
    static const MeasurementUnit Watt("Watt", []{return QObject::tr("Watt");}, []{ return QObject::tr("W"); }, 1.0);
    static const MeasurementUnit JoulePerSecond("JoulePerSecond", []{return QObject::tr("Joule Per Second");}, []{ return QObject::tr("J/s"); }, 1.0);
    static const MeasurementUnit KiloWatt("KiloWatt", []{return QObject::tr("KiloWatt");}, []{ return QObject::tr("kW"); }, 1000.0);
    static const MeasurementUnit HoursePower("HoursePower", []{return QObject::tr("Hourse Power");}, []{ return "hp"; }, 735.49875);
};

#define MEASUREMENT_POWER_STRING              MEASUREMENT_STRING(MEASUREMENT_POWER)
#define MEASUREMENT_POWER_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_POWER)
#define MEASUREMENT_POWER_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_POWER, x)
#define MEASUREMENT_POWER_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_POWER, x)
#define MEASUREMENT_POWER_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_POWER, x)
#define MEASUREMENT_POWER_DISPATCHER          MEASUREMENT_DISPATCHER(MEASUREMENT_POWER)

#endif // POWERDECLARATIONS_H
