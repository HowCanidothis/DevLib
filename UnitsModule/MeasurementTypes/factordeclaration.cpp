#include "factordeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FrictionFactorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(FrictionFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}

IMPLEMENT_MEASUREMENT(FrictionFactor, TR(MeasurementTr::tr("Friction Factor")))

namespace SeparationFactorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(SeparationFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(SeparationFactor, TR(MeasurementTr::tr("Separation Factor")))

namespace RateUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Rate, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(Rate, TR(MeasurementTr::tr("Rate")))
