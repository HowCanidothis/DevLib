#include "mudweightdeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace MudWeightUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerGallon, []{return QObject::tr("pounds per gallog");}, []{ return QObject::tr("ppg"); }, 119.8225188);
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerCubicMeter, []{return QObject::tr("kilogram per cubic meter");}, []{ return QObject::tr("kg/m3"); }, 1.0);
};

IMPLEMENT_MEASUREMENT(MudWeight)