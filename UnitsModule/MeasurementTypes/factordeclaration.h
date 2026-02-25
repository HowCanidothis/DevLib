#ifndef FACTORDECLARATION_H
#define FACTORDECLARATION_H

#include "UnitsModule/measurementdeclarations.h"

namespace FrictionFactorUnits
{
    DECLARE_MEASUREMENT_UNIT(FrictionFactor)
    DECLARE_GLOBAL(double, Min)
    DECLARE_GLOBAL(double, Max)
}
DECLARE_MEASUREMENT(FrictionFactor)

namespace SeparationFactorUnits
{
    DECLARE_MEASUREMENT_UNIT(SeparationFactor)
}
DECLARE_MEASUREMENT(SeparationFactor)

namespace BendingStressMagnificationFactorUnits
{
    DECLARE_MEASUREMENT_UNIT(BendingStressMagnificationFactor)
}
DECLARE_MEASUREMENT(BendingStressMagnificationFactor)

namespace RateUnits
{
    DECLARE_MEASUREMENT_UNIT(Rate)
}
DECLARE_MEASUREMENT(Rate)

namespace WearFactorUnits {
    DECLARE_MEASUREMENT_UNIT(Pascal)
    DECLARE_MEASUREMENT_UNIT(WearFactorSI)
    DECLARE_MEASUREMENT_UNIT(WearFactor)

    DECLARE_GLOBAL(double, Min)
    DECLARE_GLOBAL(double, Max)
}
DECLARE_MEASUREMENT(WearFactor)

namespace FatigueRatioUnits {
    DECLARE_MEASUREMENT_UNIT(FatigueRatio)
}
DECLARE_MEASUREMENT(FatigueRatio)

namespace ThermalFactorUnits {
    DECLARE_MEASUREMENT_UNIT(ThermalFactor)
}
DECLARE_MEASUREMENT(ThermalFactor)
#endif // FACTORDECLARATION_H
