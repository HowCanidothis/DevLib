#ifndef CONSISTENCYFACTORDECLARATIONS_H
#define CONSISTENCYFACTORDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace  FlowConsistencyFactorUnits
{
    DECLARE_MEASUREMENT_UNIT(FactorPascals)
    DECLARE_MEASUREMENT_UNIT(FactorPoundsPerSquareFeet)

    DECLARE_GLOBAL(double, Precision)
};

DECLARE_MEASUREMENT(FlowConsistencyFactor)

#endif // CONSISTENCYFACTORDECLARATIONS_H
