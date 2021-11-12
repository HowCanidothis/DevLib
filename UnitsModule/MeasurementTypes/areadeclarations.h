#ifndef AREADECLARATIONS_H
#define AREADECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_AREA      = "Area";

namespace AreaUnits
{
    static const MeasurementUnit SqMeters      ("SqMeters", []{return QObject::tr("square meters");}, []{ return QObject::tr("m2"); }, 1.0);
    static const MeasurementUnit SqCentimeters ("SqCentimeters", []{return QObject::tr("square centimeters");}, []{ return QObject::tr("cm2"); }, 0.0001);
    static const MeasurementUnit SqMilimeters  ("SqMilimeters", []{return QObject::tr("square milimeters");}, []{ return QObject::tr("mm2"); }, 0.000001);
    static const MeasurementUnit SqFeets       ("SqFeets", []{return QObject::tr("square feet");}, []{ return QObject::tr("ft2"); }, 1.0 / SQ_METERS_TO_FEETS_MULTIPLIER);
    static const MeasurementUnit SqUSFeets       ("SqUSFeets", []{return QObject::tr("square US feet");}, []{ return QObject::tr("usft2"); }, SQ_USFEETS_TO_FEETS_MULTIPLIER / SQ_METERS_TO_FEETS_MULTIPLIER);
    static const MeasurementUnit SqInches      ("SqInches", []{return QObject::tr("square inches");}, []{ return QObject::tr("in2"); }, 1.0 / 144.0 / SQ_METERS_TO_FEETS_MULTIPLIER);
};

#define MEASUREMENT_AREA_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_AREA)->FromUnitToBase(x)
#define MEASUREMENT_AREA_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_AREA)->FromBaseToUnit(x)
#define MEASUREMENT_AREA_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_AREA)->CurrentPrecision
#define MEASUREMENT_AREA_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_AREA_BASE_TO_UNIT(x), 'f', MEASUREMENT_AREA_PRECISION())

#define MEASUREMENT_AREA_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_AREA)->CurrentUnitLabel

#endif // AREADECLARATIONS_H
