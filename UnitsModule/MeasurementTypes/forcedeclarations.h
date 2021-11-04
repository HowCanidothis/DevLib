#ifndef FORCEDECLARATIONS_H
#define FORCEDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_FORCE = "Force";

namespace ForceUnits
{
    static const MeasurementUnit Kilonewton("Kilonewton", []{return QObject::tr("kilonewton");}, []{ return QObject::tr("kN"); }, 0.22480894);
    static const MeasurementUnit KiloPoundsForce("KiloPoundsForce", []{return QObject::tr("1000 pounds-force");}, []{ return QObject::tr("kip"); }, 1);
};

#define MEASUREMENT_FORCE_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_FORCE)->FromUnitToBase(x)
#define MEASUREMENT_FORCE_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_FORCE)->FromBaseToUnit(x)
#define MEASUREMENT_FORCE_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_FORCE)->Precision
#define MEASUREMENT_FORCE_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_FORCE_BASE_TO_UNIT(x), 'f', MEASUREMENT_FORCE_PRECISION())

#define MEASUREMENT_FORCE_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_FORCE)->CurrentUnitLabel
#define MEASUREMENT_FORCE_DISPATCHER MEASUREMENT_DISPATCHER(MEASUREMENT_FORCE)

#endif // FORCEDECLARATIONS_H
