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
}

DECLARE_MEASUREMENT(Pressure)
DECLARE_MEASUREMENT(YieldPoint)
DECLARE_MEASUREMENT(YieldStrength)
DECLARE_MEASUREMENT(YoungModulus)

namespace PressurePerDistanceUnits
{
    DECLARE_MEASUREMENT_UNIT(PascalsPerMeter)
    DECLARE_MEASUREMENT_UNIT(KilopascalsPerMeter)
    DECLARE_MEASUREMENT_UNIT(BarsPerMeter)
    DECLARE_MEASUREMENT_UNIT(MegapascalsPerMeter)
    DECLARE_MEASUREMENT_UNIT(PoundsPerSquareInchPerFeet)
    DECLARE_MEASUREMENT_UNIT(PoundsPerSquareInchPer100Feet)
    DECLARE_MEASUREMENT_UNIT(KilopoundsPerSquareInchPerFeet)
    DECLARE_MEASUREMENT_UNIT(PoundsPerSquareInchPerUSFeet)
    DECLARE_MEASUREMENT_UNIT(PoundsPerSquareInchPer100USFeet)
    DECLARE_MEASUREMENT_UNIT(KilopoundsPerSquareInchPerUSFeet)
}

DECLARE_MEASUREMENT(PressurePerDistance)
#endif // PRESSUREDECLARATIONS_H
