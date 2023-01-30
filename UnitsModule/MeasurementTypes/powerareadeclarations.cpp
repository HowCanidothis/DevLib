#include "powerareadeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace PowerAreaUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(WattPerSqMeter, []{return QObject::tr("Watt Per Square Meters");}, []{ return QObject::tr("W/m2"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(HoursePowerPerSqMeter, []{return QObject::tr("Hourse Power Per Square Meters");}, []{ return QObject::tr("hp/m2"); }, 735.49875);
    IMPLEMENT_MEASUREMENT_UNIT(HoursePowerPerSqInch, []{return QObject::tr("Hourse Power Per Square Inch");}, []{ return QObject::tr("hp/in2"); }, 735.49875/1550.003100006);
};

IMPLEMENT_MEASUREMENT(PowerArea, TR(MeasurementTr::tr("PowerArea")))
