#include "consistencyfactordeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FlowConsistencyFactorUnits {
    IMPLEMENT_MEASUREMENT_UNIT(FactorPascals,               TR(MeasurementTr::tr("factor*pascals")),                TR(MeasurementTr::tr("Pa*s^n")),        1.0)
    IMPLEMENT_MEASUREMENT_UNIT(FactorPoundsPerSquareFeet,   TR(MeasurementTr::tr("factor*pounds-force/sq.feet")),   TR(MeasurementTr::tr("lb*s^n/ft2")),    47.880172)
}

IMPLEMENT_MEASUREMENT(FlowConsistencyFactor, TR(MeasurementTr::tr("Flow Consistency Factor")))
