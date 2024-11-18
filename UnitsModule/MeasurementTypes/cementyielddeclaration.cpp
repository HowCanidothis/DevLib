#include "cementyielddeclaration.h"
#include "UnitsModule/measurementunitmanager.h"

namespace CementYieldUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerKilogram, TR(MeasurementTr::tr("cubic meters / kilograms")),    TR(MeasurementTr::tr("m3/kg")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerTonne, TR(MeasurementTr::tr("cubic meters / tonne")),    TR(MeasurementTr::tr("m3/tonne")),     0.001)
    IMPLEMENT_MEASUREMENT_UNIT(CubicFootPerSack, TR(MeasurementTr::tr("cubic foot / sack")),    TR(MeasurementTr::tr("ft3/sack")),     0.00150573568)
};
IMPLEMENT_MEASUREMENT(CementYield, TR(MeasurementTr::tr("Cement Yield")))
