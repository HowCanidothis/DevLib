#ifndef PRESSUREDECLARATIONS_H
#define PRESSUREDECLARATIONS_H

#include "UnitsModule/measurementdeclarations.h"

#define YieldPointUnits PressureUnits
#define YieldStrengthUnits PressureUnits
#define YoungModulusUnits PressureUnits

namespace PressureUnits
{
    DECLARE_MEASUREMENT_UNIT(Pascals)
    DECLARE_MEASUREMENT_UNIT(Kilopascals)
    DECLARE_MEASUREMENT_UNIT(NewtonPerSquareMeter)
    DECLARE_MEASUREMENT_UNIT(Bars)
    DECLARE_MEASUREMENT_UNIT(Megapascals)
    DECLARE_MEASUREMENT_UNIT(Atmospheres)
    DECLARE_MEASUREMENT_UNIT(KilogramPerSquareCentimeter)
    DECLARE_MEASUREMENT_UNIT(KilogramPerSquareMeter)
    DECLARE_MEASUREMENT_UNIT(PoundsPerSquareInch)
    DECLARE_MEASUREMENT_UNIT(KilopoundsPerSquareInch)
    DECLARE_MEASUREMENT_UNIT(PoundsPerSquareFeet)
    DECLARE_MEASUREMENT_UNIT(PoundsPer100SquareFeet)

    DECLARE_GLOBAL(double, FatigueEnduranceLimit)
}

DECLARE_MEASUREMENT(Pressure)
DECLARE_MEASUREMENT(YieldPoint)
DECLARE_MEASUREMENT(YieldStrength)
DECLARE_MEASUREMENT(YoungModulus)


#endif // PRESSUREDECLARATIONS_H
