#include "angledeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace AngleUnits {
IMPLEMENT_MEASUREMENT_UNIT(Radians, TR(MeasurementTr::tr("radians")), TR(MeasurementTr::tr("rad")), 1.0)
IMPLEMENT_MEASUREMENT_UNIT(Degrees, TR(MeasurementTr::tr("degrees")), TR(MeasurementTr::tr("°")), DEGREES_TO_RADIANS)
}

IMPLEMENT_MEASUREMENT(Angle, TR(MeasurementTr::tr("Angles")))
