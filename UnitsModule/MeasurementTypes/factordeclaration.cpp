#include "factordeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FrictionFactorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(FrictionFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)

    IMPLEMENT_GLOBAL(double, Min, 0.01);
    IMPLEMENT_GLOBAL(double, Max, 3.0);
}

IMPLEMENT_MEASUREMENT(FrictionFactor, TR(MeasurementTr::tr("Friction Factor")))

namespace SeparationFactorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(SeparationFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(SeparationFactor, TR(MeasurementTr::tr("Separation Factor")))

namespace BendingStressMagnificationFactorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(BendingStressMagnificationFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(BendingStressMagnificationFactor, TR(MeasurementTr::tr("Bending Stress Magnification Factor")))

namespace RateUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Rate, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(Rate, TR(MeasurementTr::tr("Rate")))

namespace WearFactorUnits {
    IMPLEMENT_MEASUREMENT_UNIT(Pascal, TR(MeasurementTr::tr("1 / Pa")), TR(MeasurementTr::tr("Pa-1")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(WearFactorSI, TR(MeasurementTr::tr("E-10 / kPa")), TR(MeasurementTr::tr("E-10 / kPa")), 1.e-13)
    IMPLEMENT_MEASUREMENT_UNIT(WearFactor, TR(MeasurementTr::tr("E-10 / psi")), TR(MeasurementTr::tr("E-10 / psi")), 1.e-10 / 6894.757)
}
IMPLEMENT_MEASUREMENT(WearFactor, TR(MeasurementTr::tr("Wear Factor")))

namespace FatigueRatioUnits {
    IMPLEMENT_MEASUREMENT_UNIT(FatigueRatio, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(FatigueRatio, TR(MeasurementTr::tr("Fatigue Ratio")))

namespace ThermalFactorUnits {
    IMPLEMENT_MEASUREMENT_UNIT(ThermalFactor, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}
IMPLEMENT_MEASUREMENT(ThermalFactor, TR(MeasurementTr::tr("Thermal Factor")))
