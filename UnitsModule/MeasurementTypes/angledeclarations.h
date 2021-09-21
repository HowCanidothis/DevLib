#ifndef ANGLEDECLARATIONS_H
#define ANGLEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_ANGLES = "Angle";

namespace AngleUnits
{
    static const MeasurementUnit Radians("Radian", []{return QObject::tr("radians");}, []{ return QObject::tr("rad"); }, 1.0);
    static const MeasurementUnit Degrees("Degree", []{return QObject::tr("degrees");}, []{ return "°"; }, DEGREES_TO_RADIANS);
};

#define MEASUREMENT_ANGLES_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_ANGLES)->FromUnitToBase(x)
#define MEASUREMENT_ANGLES_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_ANGLES)->FromBaseToUnit(x)
#define MEASUREMENT_ANGLES_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_ANGLES)->Precision
#define MEASUREMENT_ANGLES_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_ANGLES_BASE_TO_UNIT(x), 'f', MEASUREMENT_ANGLES_PRECISION())

#define MEASUREMENT_ANGLES_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_ANGLES)->CurrentUnitLabel
#define ATTACH_ANGLES_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_ANGLES, delegate, min, max, 1)

#endif // ANGLEDECLARATIONS_H
