#ifndef TIMEDECLARATIONS_H
#define TIMEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_TIME = "Time";

namespace TimeUnits
{
    static const MeasurementUnit MSeconds("MSeconds",   []{return QObject::tr("msec");}, []{ return QObject::tr("ms"); }, 1.0/1000);
    static const MeasurementUnit Seconds ("Seconds",    []{return QObject::tr("sec");}, []{ return QObject::tr("s"); }, 1);
    static const MeasurementUnit Minutes ("Minutes",    []{return QObject::tr("min");}, []{ return QObject::tr("m"); }, 60);
    static const MeasurementUnit Hours   ("Hours",      []{return QObject::tr("hour");}, []{ return QObject::tr("h"); }, 3600);
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
#define ATTACH_TIME_MEASUREMENT(delegate, min, max, step) \
    ATTACH_MEASUREMENT(MEASUREMENT_TIME, delegate, min, max, step)


#endif // TIMEDECLARATIONS_H
