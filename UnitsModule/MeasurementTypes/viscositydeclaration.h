#ifndef VISCOSITYDECLARATIONS_H
#define VISCOSITYDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_VISCOSITY = "Viscosity";

namespace ViscosityUnits
{
    static const MeasurementUnit PascalsPerSecond("PascalsPerSecond", []{return QObject::tr("pascal-second");}, []{ return QObject::tr("Pa*s"); }, 1.0);
    static const MeasurementUnit Centipoise("Centipoise", []{return QObject::tr("centipoise");}, []{ return QObject::tr("cp"); }, 0.001);
    static const MeasurementUnit MilliPascalSecond("Millipascal-second", []{return QObject::tr("millipascal-second");}, []{ return QObject::tr("mPa*s"); }, 0.001);
};

#define MEASUREMENT_VISCOSITY_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_VISCOSITY)->FromUnitToBase(x)
#define MEASUREMENT_VISCOSITY_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_VISCOSITY)->FromBaseToUnit(x)
#define MEASUREMENT_VISCOSITY_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_VISCOSITY)->CurrentPrecision
#define MEASUREMENT_VISCOSITY_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_VISCOSITY_BASE_TO_UNIT(x), 'f', MEASUREMENT_VISCOSITY_PRECISION())

#define MEASUREMENT_VISCOSITY_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_VISCOSITY)->CurrentUnitLabel

#endif // VISCOSITYDECLARATIONS_H
