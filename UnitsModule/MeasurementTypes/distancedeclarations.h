#ifndef DISTANCEDECLARATIONS_H
#define DISTANCEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_DISTANCES      = "Length";
static const Name MEASUREMENT_DIAMETER       = "Bit Diameter";

namespace DistanceUnits
{
    static const MeasurementUnit Meters      ("Meters", []{return QObject::tr("meters");}, []{ return QObject::tr("m"); }, 3.280839895);
    static const MeasurementUnit Milimeters  ("Milimeters", []{return QObject::tr("milimeters");}, []{ return QObject::tr("mm"); }, 0.003280839895);
    static const MeasurementUnit Centimeters ("Centimeters", []{return QObject::tr("centimeters");}, []{ return QObject::tr("cm"); }, 0.03280839895);
    static const MeasurementUnit Kilometers  ("Kilometers", []{return QObject::tr("kilometers");}, []{ return QObject::tr("km"); }, 3280.839895);
    static const MeasurementUnit Feets       ("Feets"  , []{return QObject::tr("feet");},  []{ return QObject::tr("ft"); }, 1.0);
    static const MeasurementUnit USFeets     ("Usfeets", []{return QObject::tr("US feet");},[]{ return QObject::tr("usft"); }, USFEETS_TO_FEETS_MULTIPLIER);
    static const MeasurementUnit Inches      ("Inches", []{return QObject::tr("inches");},[]{ return QObject::tr("in"); }, 0.0833333);
    static const MeasurementUnit Miles       ("Miles", []{return QObject::tr("miles");},[]{ return QObject::tr("miles"); }, 5280);
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
#define ATTACH_DISTANCE_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_DISTANCES, delegate, min, max)

#define MEASUREMENT_DIAMETER_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DIAMETER)->FromUnitToBase(x)
#define MEASUREMENT_DIAMETER_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DIAMETER)->FromBaseToUnit(x)
#define MEASUREMENT_DIAMETER_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DIAMETER)->Precision
#define MEASUREMENT_DIAMETER_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_DIAMETER_BASE_TO_UNIT(x), 'f', MEASUREMENT_DIAMETER_PRECISION())

#define MEASUREMENT_DIAMETER_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DIAMETER)->CurrentUnitLabel
#define ATTACH_DIAMETER_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_DIAMETER, delegate, min, max)

#endif // DISTANCEDECLARATIONS_H