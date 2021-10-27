#ifndef YIELDSTRENGHTDECLARATIONS_H
#define YIELDSTRENGHTDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_YIELD_STRENGTH = "Yield Strength";

namespace YieldStrengthUnits
{
    static const MeasurementUnit PoundsPerSquareInch("PoundsPerSquareInch", []{return QObject::tr("pounds-force/sq.inch");}, []{ return QObject::tr("psi"); }, 1.0);
    static const MeasurementUnit Kilopascals("Kilopascals", []{return QObject::tr("kilopascals");}, []{ return QObject::tr("kPa"); }, 0.1450377);
};

#define MEASUREMENT_YIELD_STRENGTH_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_YIELD_STRENGTH)->FromUnitToBase(x)
#define MEASUREMENT_YIELD_STRENGTH_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_YIELD_STRENGTH)->FromBaseToUnit(x)
#define MEASUREMENT_YIELD_STRENGTH_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_YIELD_STRENGTH)->Precision
#define MEASUREMENT_YIELD_STRENGTH_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_YIELD_STRENGTH_BASE_TO_UNIT(x), 'f', MEASUREMENT_YIELD_STRENGTH_PRECISION())

#define MEASUREMENT_YIELD_STRENGTH_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_YIELD_STRENGTH)->CurrentUnitLabel
#define ATTACH_YIELD_STRENGTH_MEASUREMENT(delegate, min, max, step) \
    ATTACH_MEASUREMENT(MEASUREMENT_YIELD_STRENGTH, delegate, min, max, step)

#endif // YIELDSTRENGHTDECLARATIONS_H
