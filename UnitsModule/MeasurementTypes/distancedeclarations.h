#ifndef DISTANCEDECLARATIONS_H
#define DISTANCEDECLARATIONS_H

#include "measurementunitmanager.h"

static const Name MEASUREMENT_DISTANCES      = "Length";

namespace DistanceUnits
{
    static const MeasurementUnit Meters ("Meters", []{return QObject::tr("meters");}, []{ return QObject::tr("m"); }, 3.280839895);
    static const MeasurementUnit Feets  ("Feets"  , []{return QObject::tr("feet");},  []{ return QObject::tr("ft"); }, 1.0);
    static const MeasurementUnit USFeets("Usfeets", []{return QObject::tr("US feet");},[]{ return QObject::tr("usft"); }, USFEETS_TO_FEETS_MULTIPLIER);
};

#define MEASUREMENT_DISTANCE_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DISTANCES)->FromUnitToBase(x)
#define MEASUREMENT_DISTANCE_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DISTANCES)->FromBaseToUnit(x)
#define MEASUREMENT_DISTANCE_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DISTANCES)->Precision
#define MEASUREMENT_DISTANCE_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_DISTANCE_BASE_TO_UNIT(x), 'f', MEASUREMENT_DISTANCE_PRECISION())

#define MEASUREMENT_DISTANCE_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DISTANCES)->CurrentUnitLabel

#endif // DISTANCEDECLARATIONS_H
