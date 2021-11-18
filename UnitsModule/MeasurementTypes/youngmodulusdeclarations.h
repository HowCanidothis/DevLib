#ifndef YOUNGMODULUSDECLARATIONS_H
#define YOUNGMODULUSDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_YOUNG_MODULUS = "Young's Modulus";

namespace YoungModulusUnits
{
    // Using Pressure
};

#define MEASUREMENT_YOUNG_MODULUS_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_YOUNG_MODULUS)->FromUnitToBase(x)
#define MEASUREMENT_YOUNG_MODULUS_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_YOUNG_MODULUS)->FromBaseToUnit(x)
#define MEASUREMENT_YOUNG_MODULUS_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_YOUNG_MODULUS)->CurrentPrecision
#define MEASUREMENT_YOUNG_MODULUS_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_YOUNG_MODULUS_BASE_TO_UNIT(x), 'f', MEASUREMENT_YOUNG_MODULUS_PRECISION())

#define MEASUREMENT_YOUNG_MODULUS_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_YOUNG_MODULUS)->CurrentUnitLabel
#define ATTACH_YOUNG_MODULUS_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_YOUNG_MODULUS, delegate, min, max)

#endif // YOUNGMODULUSDECLARATIONS_H
