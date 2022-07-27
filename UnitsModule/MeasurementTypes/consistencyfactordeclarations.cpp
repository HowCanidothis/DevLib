#include "consistencyfactordeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FlowConsistencyFactorUnits {
    IMPLEMENT_MEASUREMENT_UNIT(FactorPascals, []{return QObject::tr("factor*pascals");}, []{ return QObject::tr("Pa*s^n"); }, 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(FactorPoundsPerSquareFeet, []{return QObject::tr("factor*pounds-force/sq.feet");}, []{ return QObject::tr("lb*s^n/ft2"); }, 47.880172)
}

IMPLEMENT_MEASUREMENT(FlowConsistencyFactor)
