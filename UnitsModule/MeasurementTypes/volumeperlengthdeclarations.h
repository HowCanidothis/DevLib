#ifndef VOLUMEPERLENGTHDECLARATIONS_H
#define VOLUMEPERLENGTHDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_VOLUME_PER_LENGTH = "Volume Per Length";
static const Name MEASUREMENT_LINEAR_CAPACITY = "Linear Capacity";

namespace VolumePerLengthUnits
{
    static const MeasurementUnit CubicMeterPerMeter("CubicMeterPerMeter", []{return QObject::tr("cubic meter per meter");}, []{ return QObject::tr("m3/m"); }, 1);
    static const MeasurementUnit BarrelPerFoot("BarrelPerFoot", []{return QObject::tr("barrel per foot");}, []{ return QObject::tr("bbl/ft"); }, 0.52161187664042);
    static const MeasurementUnit LiterPerMeter("LiterPerMeter", []{return QObject::tr("liter per meter");}, []{ return QObject::tr("L/m"); }, 0.001);
};

#define MEASUREMENT_LINEAR_CAPACITY_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_LINEAR_CAPACITY)->FromUnitToBase(x)
#define MEASUREMENT_LINEAR_CAPACITY_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_LINEAR_CAPACITY)->FromBaseToUnit(x)
#define MEASUREMENT_LINEAR_CAPACITY_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_LINEAR_CAPACITY)->CurrentPrecision
#define MEASUREMENT_LINEAR_CAPACITY_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_LINEAR_CAPACITY_BASE_TO_UNIT(x), 'f', MEASUREMENT_LINEAR_CAPACITY_PRECISION())

#define MEASUREMENT_LINEAR_CAPACITY_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_LINEAR_CAPACITY)->CurrentUnitLabel
#define MEASUREMENT_LINEAR_CAPACITY_DISPATCHER MEASUREMENT_DISPATCHER(MEASUREMENT_LINEAR_CAPACITY)

#endif // VOLUMEPERLENGTHDECLARATIONS_H
