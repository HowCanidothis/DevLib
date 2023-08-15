#include "momentofinertia.h"
#include "UnitsModule/measurementunitmanager.h"

namespace MomentOfInertiaUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogrammSqMeters,       TR(MeasurementTr::tr("Kilogramm Square Second")),       TR(MeasurementTr::tr("kg·m2")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundSqFoot,             TR(MeasurementTr::tr("Pound Square Foot")),             TR(MeasurementTr::tr("lbm·ft2")),   0.0421401100938)
    IMPLEMENT_MEASUREMENT_UNIT(PoundForceFootSqSecond,  TR(MeasurementTr::tr("Pound Force Foot Square Second")),TR(MeasurementTr::tr("lbf·ft·s2")), 1.355817961893)
}

IMPLEMENT_MEASUREMENT(MomentOfInertia, TR(MeasurementTr::tr("Moment Of Inertia")))
