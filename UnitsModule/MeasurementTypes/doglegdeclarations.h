#ifndef DOGLEGDECLARATIONS_H
#define DOGLEGDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_DLS = "Dogleg Severity";

namespace DLSUnits
{
    static const MeasurementUnit RadFeet     ("RadianFeet"  , []{return QObject::tr("rad per 100ft");}, []{ return QObject::tr("rad/100ft"); }, (30.0 * METERS_TO_FEETS_MULTIPLIER) / 100.0);
    static const MeasurementUnit RadMeter    ("RadianMeter" , []{return QObject::tr("rad per 30m");}, []{ return QObject::tr("rad/30m"); }, 1.0);
    static const MeasurementUnit DegreeFeet  ("DegreeFeet"  , []{return QObject::tr("deg per 100ft");}, []{ return QObject::tr("°/100ft"); }, DEGREES_TO_RADIANS * (30.0 * METERS_TO_FEETS_MULTIPLIER) / 100.0);
    static const MeasurementUnit DegreeUSFeet("DegreeUSFeet", []{return QObject::tr("deg per 100usft");}, []{ return QObject::tr("°/100usft"); }, DEGREES_TO_RADIANS * (30.0 * METERS_TO_FEETS_MULTIPLIER) / 100.0 / USFEETS_TO_FEETS_MULTIPLIER);
    static const MeasurementUnit DegreeMeter ("DegreeMeter" , []{return QObject::tr("deg per 30m");}, []{ return QObject::tr("°/30m"); }, DEGREES_TO_RADIANS);
}

#define MEASUREMENT_DLS_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DLS)->FromUnitToBase(x)
#define MEASUREMENT_DLS_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_DLS)->FromBaseToUnit(x)
#define MEASUREMENT_DLS_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DLS)->CurrentPrecision
#define MEASUREMENT_DLS_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_DLS_BASE_TO_UNIT(x), 'f', MEASUREMENT_DLS_PRECISION())

#define MEASUREMENT_DLS_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_DLS)->CurrentUnitLabel

#endif // DOGLEGDECLARATIONS_H
