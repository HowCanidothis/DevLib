#ifndef TIMEDECLARATIONS_H
#define TIMEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_TIME = "Time";

namespace TimeUnits
{
    static const MeasurementUnit MSeconds("MSeconds",   []{return QObject::tr("msec");}, []{ return QObject::tr("ms"); }, 0.001);
    static const MeasurementUnit Seconds ("Seconds",    []{return QObject::tr("sec");}, []{ return QObject::tr("s"); }, 1.0);
    static const MeasurementUnit Minutes ("Minutes",    []{return QObject::tr("min");}, []{ return QObject::tr("min"); }, 60.0);
    static const MeasurementUnit Hours   ("Hours",      []{return QObject::tr("hour");}, []{ return QObject::tr("hr"); }, 3600.0);
    static const MeasurementUnit Day   ("Day",      []{return QObject::tr("days");}, []{ return QObject::tr("day"); }, 86400.0);
    static const MeasurementUnit Months   ("Months",      []{return QObject::tr("months");}, []{ return QObject::tr("month"); }, 2628000.0);
    static const MeasurementUnit Years   ("Years",      []{return QObject::tr("years");}, []{ return QObject::tr("year"); }, 31536000.0);
};

#define MEASUREMENT_TIME_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_TIME)->FromUnitToBase(x)
#define MEASUREMENT_TIME_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_TIME)->FromBaseToUnit(x)
#define MEASUREMENT_TIME_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_TIME)->CurrentPrecision
#define MEASUREMENT_TIME_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_TIME_BASE_TO_UNIT(x), 'f', MEASUREMENT_TIME_PRECISION())

#define MEASUREMENT_TIME_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_TIME)->CurrentUnitLabel
#define MEASUREMENT_TIME_DISPATCHER MEASUREMENT_DISPATCHER(MEASUREMENT_TIME)
#define ATTACH_TIME_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_TIME, delegate, min, max)


#endif // TIMEDECLARATIONS_H
