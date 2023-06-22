#ifndef CONCENTRATIONDECLARATION_H
#define CONCENTRATIONDECLARATION_H

#include "UnitsModule/measurementdeclarations.h"

namespace ConcentrationUnits {
    DECLARE_MEASUREMENT_UNIT(MilliliterPerMilliliter)
    DECLARE_MEASUREMENT_UNIT(PartsPerBillion)
    DECLARE_MEASUREMENT_UNIT(CubicMetersPerCubicMeters)
    DECLARE_MEASUREMENT_UNIT(KilogrammPerCubicMeters)
    DECLARE_MEASUREMENT_UNIT(MilligrammPerLiter)
    DECLARE_MEASUREMENT_UNIT(GallonPerMegagallon)
};

DECLARE_MEASUREMENT(Concentration)
DECLARE_MEASUREMENT(ConcentrationMud)
DECLARE_MEASUREMENT(ConcentrationAlkalinity)

#endif // CONCENTRATIONDECLARATION_H
