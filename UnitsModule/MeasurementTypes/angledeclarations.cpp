#include "angledeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace AngleUnits {
IMPLEMENT_MEASUREMENT_UNIT(Radians, []{return QObject::tr("radians");}, []{ return QObject::tr("rad"); }, 1.0)
IMPLEMENT_MEASUREMENT_UNIT(Degrees, []{return QObject::tr("degrees");}, []{ return "°"; }, DEGREES_TO_RADIANS)
}

IMPLEMENT_MEASUREMENT(MeasurementAngle)
