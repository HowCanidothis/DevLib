#include "viscositydeclaration.h"
#include "UnitsModule/measurementunitmanager.h"

namespace ViscosityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(PascalsPerSecond, []{return QObject::tr("pascal-second");}, []{ return QObject::tr("Pa*s"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Centipoise, []{return QObject::tr("centipoise");}, []{ return QObject::tr("cp"); }, 0.001);
    IMPLEMENT_MEASUREMENT_UNIT(MilliPascalSecond, []{return QObject::tr("millipascal-second");}, []{ return QObject::tr("mPa*s"); }, 0.001);
};

IMPLEMENT_MEASUREMENT(Viscosity)
