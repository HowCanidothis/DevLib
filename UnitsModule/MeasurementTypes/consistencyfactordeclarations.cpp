#include "consistencyfactordeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FlowConsistencyFactorUnits {
    IMPLEMENT_MEASUREMENT_UNIT(FactorPascals,               TR(MeasurementTr::tr("factor*pascals")),                TR(MeasurementTr::tr("Pa∙s^n")),        1.0)
    IMPLEMENT_MEASUREMENT_UNIT(FactorPoundsPerSquareFeet,   TR(MeasurementTr::tr("factor*pounds-force/sq.feet")),   TR(MeasurementTr::tr("lb∙s^n/ft2")),    47.880172)

    // Допуск для fuzzyIsNull(K): 1e-5 Pa·s^n ≈ 0.01 cP — на 2 порядка ниже вязкости воды (Newtonian: K = PV ≈ 1e-3 Pa·s)
    // и меньше минимума K в UI (0.0001 lb·s^n/ft² ≈ 4.8e-3 Pa·s^n). WPS_EPSILON (0.003) и точность отображения не подходят:
    // обе больше вязкости воды.
    IMPLEMENT_GLOBAL(double, Precision, FactorPascals.FromUnitToBase(1e-5));
}

IMPLEMENT_MEASUREMENT(FlowConsistencyFactor, TR(MeasurementTr::tr("Flow Consistency Factor")))
