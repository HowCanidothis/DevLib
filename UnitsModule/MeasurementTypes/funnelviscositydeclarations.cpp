#include "funnelviscositydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FunnelViscosityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(SecondsForCubicMeter, []{return QObject::tr("seconds/cubic meter");}, []{ return QObject::tr("sec/m3"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(SecondsForQuart, []{return QObject::tr("seconds/quart");}, []{ return QObject::tr("sec/qt"); }, 1.0 / 0.000946353);
};

IMPLEMENT_MEASUREMENT(FunnelViscosity)
