#include "momentofinertia.h"
#include "UnitsModule/measurementunitmanager.h"

namespace MomentOfInertiaUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogrammSqMeters,       []{return QObject::tr("Kilogramm Square Second");},             []{ return QObject::tr("kg·m2");      },     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundSqFoot,             []{return QObject::tr("Pound Square Foot");},                   []{ return QObject::tr("lbm·ft2");      },   23.73036040423)
    IMPLEMENT_MEASUREMENT_UNIT(PoundForceFootSqSecond,  []{return QObject::tr("Pound Force Foot Square Second");},      []{ return QObject::tr("lbf·ft·s2");      },     0.7375621419)
}

IMPLEMENT_MEASUREMENT(MomentOfInertia, TR(MeasurementTr::tr("Moment Of Inertia")))
