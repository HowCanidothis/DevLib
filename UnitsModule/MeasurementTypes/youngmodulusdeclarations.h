#ifndef YOUNGMODULUSDECLARATIONS_H
#define YOUNGMODULUSDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_YOUNG_MODULUS = "Young Modulus";

namespace YoungModulusUnits
{
    static const MeasurementUnit PoundsPerSquareInch("PoundsPerSquareInch", []{return QObject::tr("pounds-force/sq.inch");}, []{ return QObject::tr("psi"); }, 1.0);
    static const MeasurementUnit Kilopascals("Kilopascals", []{return QObject::tr("kilopascals");}, []{ return QObject::tr("kPa"); }, 0.1450377);
};

#define MEASUREMENT_YOUNG_MODULUS_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_YOUNG_MODULUS)->FromUnitToBase(x)
#define MEASUREMENT_YOUNG_MODULUS_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_YOUNG_MODULUS)->FromBaseToUnit(x)
#define MEASUREMENT_YOUNG_MODULUS_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_YOUNG_MODULUS)->Precision
#define MEASUREMENT_YOUNG_MODULUS_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_YOUNG_MODULUS_BASE_TO_UNIT(x), 'f', MEASUREMENT_YOUNG_MODULUS_PRECISION())

#define MEASUREMENT_YOUNG_MODULUS_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_YOUNG_MODULUS)->CurrentUnitLabel
#define ATTACH_YOUNG_MODULUS_MEASUREMENT(delegate, min, max, step) \
    ATTACH_MEASUREMENT(MEASUREMENT_YOUNG_MODULUS, delegate, min, max, step)

#endif // YOUNGMODULUSDECLARATIONS_H
