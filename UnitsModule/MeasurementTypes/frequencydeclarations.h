#ifndef FREQUENCYDECLARATIONS_H
#define FREQUENCYDECLARATIONS_H


#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_FRENQUENCY = "Frequency";

namespace FrequencyUnits
{
    static const MeasurementUnit RevolutionPerMinute("RevolutionPerMinute", []{return QObject::tr("rpm");}, []{ return QObject::tr("rpm"); }, 1.0);
//    static const MeasurementUnit RevolutionPerSecond("RevolutionPerSecond", []{return QObject::tr("rps");}, []{ return QObject::tr("1/sec"); }, 1.0);
};

#define MEASUREMENT_FRENQUENCY_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_FRENQUENCY)->FromUnitToBase(x)
#define MEASUREMENT_FRENQUENCY_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_FRENQUENCY)->FromBaseToUnit(x)
#define MEASUREMENT_FRENQUENCY_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_FRENQUENCY)->CurrentPrecision
#define MEASUREMENT_FRENQUENCY_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_FRENQUENCY_BASE_TO_UNIT(x), 'f', MEASUREMENT_FRENQUENCY_PRECISION())

#define MEASUREMENT_FRENQUENCY_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_FRENQUENCY)->CurrentUnitLabel
#define MEASUREMENT_FRENQUENCY_DISPATCHER MEASUREMENT_DISPATCHER(MEASUREMENT_FRENQUENCY)
#define ATTACH_FRENQUENCY_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_FRENQUENCY, delegate, min, max)

#endif // FREQUENCYDECLARATIONS_H
