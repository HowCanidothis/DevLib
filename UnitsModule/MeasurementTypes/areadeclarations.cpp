#include "areadeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace AreaUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(SqMeters,        TR(MeasurementTr::tr("square meters")),     TR(MeasurementTr::tr("m2")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(SqCentimeters,   TR(MeasurementTr::tr("square centimeters")),TR(MeasurementTr::tr("cm2")), 0.0001)
    IMPLEMENT_MEASUREMENT_UNIT(SqMilimeters,    TR(MeasurementTr::tr("square milimeters")), TR(MeasurementTr::tr("mm2")), 0.000001)
    IMPLEMENT_MEASUREMENT_UNIT(SqFeets,         TR(MeasurementTr::tr("square feet")),       TR(MeasurementTr::tr("ft2")), 1.0 / SQ_METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(SqUSFeets,       TR(MeasurementTr::tr("square US feet")),    TR(MeasurementTr::tr("usft2")), SQ_USFEETS_TO_FEETS_MULTIPLIER / SQ_METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(SqInches,        TR(MeasurementTr::tr("square inches")),     TR(MeasurementTr::tr("in2")), 1.0 / 144.0 / SQ_METERS_TO_FEETS_MULTIPLIER)
}

IMPLEMENT_MEASUREMENT(Area, TR(MeasurementTr::tr("Area")))
