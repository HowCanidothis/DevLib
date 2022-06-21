#include "percentsdeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace PercentsUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Partial, []{return QObject::tr("partial"); }, []{ return QObject::tr("");  }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Percents, []{return QObject::tr("percents");     }, []{ return "%";   }, 0.01);
}

IMPLEMENT_MEASUREMENT(Percents)
