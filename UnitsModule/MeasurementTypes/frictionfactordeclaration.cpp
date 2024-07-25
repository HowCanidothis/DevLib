#include "frictionfactordeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FrictionFactorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(FrictionFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}

IMPLEMENT_MEASUREMENT(FrictionFactor, TR(MeasurementTr::tr("Friction Factor")))
