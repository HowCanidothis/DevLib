#ifndef ANGLEDECLARATIONS_H
#define ANGLEDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

namespace AngleUnits
{
    DECLARE_MEASUREMENT_UNIT(Radians)
    DECLARE_MEASUREMENT_UNIT(Degrees)
    DECLARE_MEASUREMENT_UNIT(NumberOfRevolutions)
};

DECLARE_MEASUREMENT(Angle)
DECLARE_MEASUREMENT(PlaneAngle)

#endif // ANGLEDECLARATIONS_H
