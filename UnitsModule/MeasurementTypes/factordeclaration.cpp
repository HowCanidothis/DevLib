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

namespace WearFactorUnits {
    IMPLEMENT_MEASUREMENT_UNIT(Pascal, TR(MeasurementTr::tr("1 / Pa")), TR(MeasurementTr::tr("Pa-1")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(WearFactor, TR(MeasurementTr::tr("E-10 / psi")), TR(MeasurementTr::tr("E-10 / psi")), 1.e-10 / 6894.757)
}
IMPLEMENT_MEASUREMENT(WearFactor, TR(MeasurementTr::tr("Wear Factor")))
