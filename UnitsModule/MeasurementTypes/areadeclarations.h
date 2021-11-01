#ifndef AREADECLARATIONS_H
#define AREADECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_AREA      = "AREA";
static constexpr double SQ_METERS_TO_SQ_FEETS_MULTIPLIER = METERS_TO_FEETS_MULTIPLIER*METERS_TO_FEETS_MULTIPLIER;

namespace AreaUnits
{
    static const MeasurementUnit SqMeters      ("SqMeters", []{return QObject::tr("square meters");}, []{ return QObject::tr("m2"); }, SQ_METERS_TO_SQ_FEETS_MULTIPLIER);
    static const MeasurementUnit SqCentimeters ("SqCentimeters", []{return QObject::tr("square centimeters");}, []{ return QObject::tr("cm2"); }, SQ_METERS_TO_SQ_FEETS_MULTIPLIER/10e4);
    static const MeasurementUnit SqMilimeters  ("SqMilimeters", []{return QObject::tr("square milimeters");}, []{ return QObject::tr("mm2"); }, SQ_METERS_TO_SQ_FEETS_MULTIPLIER/10e6);
    static const MeasurementUnit SqFeets       ("SqFeets", []{return QObject::tr("square feet");}, []{ return QObject::tr("ft2"); }, 1);
    static const MeasurementUnit SqInches      ("SqInches", []{return QObject::tr("square inches");}, []{ return QObject::tr("in2"); }, [](double v){ return v/144.0; }, [](double v){ return v*144.0;});
};

#define MEASUREMENT_AREA_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_AREA)->FromUnitToBase(x)
#define MEASUREMENT_AREA_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_AREA)->FromBaseToUnit(x)
#define MEASUREMENT_AREA_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_AREA)->Precision
#define MEASUREMENT_AREA_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_AREA_BASE_TO_UNIT(x), 'f', MEASUREMENT_AREA_PRECISION())

#define MEASUREMENT_AREA_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_AREA)->CurrentUnitLabel

#endif // AREADECLARATIONS_H
