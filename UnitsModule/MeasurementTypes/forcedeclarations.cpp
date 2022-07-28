#include "forcedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace ForceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Newton, []{return QObject::tr("newton");}, []{ return QObject::tr("N"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Kilonewton, []{return QObject::tr("kilonewton");}, []{ return QObject::tr("kN"); }, 1000.0);
    IMPLEMENT_MEASUREMENT_UNIT(PoundsForce, []{return QObject::tr("pounds-force");}, []{ return QObject::tr("lbf"); }, 4.44822);
    IMPLEMENT_MEASUREMENT_UNIT(KiloGrammForce, []{return QObject::tr("kilogram-force");}, []{ return QObject::tr("kgf"); }, 9.806650029);
    IMPLEMENT_MEASUREMENT_UNIT(KiloPoundsForce, []{return QObject::tr("1000 pounds-force");}, []{ return QObject::tr("kip"); }, 4448.22);
};

IMPLEMENT_MEASUREMENT(Force)
