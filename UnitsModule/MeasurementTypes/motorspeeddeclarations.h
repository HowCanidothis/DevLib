#ifndef MOTORSPEEDDECLARATIONS_H
#define MOTORSPEEDDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_MOTOR_SPEED = "Motor speed";

namespace MotorSpeedUnits
{
    static const MeasurementUnit RevolutionPerGallon("RevolutionPerGallon", []{return QObject::tr("revolution per gallon");}, []{ return QObject::tr("rev/gal"); }, 264.172052);
    static const MeasurementUnit RevolutionPerLiter("RevolutionPerLiter", []{return QObject::tr("revolution per liter");}, []{ return QObject::tr("rev/l"); }, 1000.0);
    static const MeasurementUnit RevolutionPerCubicMeter("RevolutionPerCubicMeter", []{return QObject::tr("revolution per cubic meter");}, []{ return QObject::tr("rev/m3"); }, 1.0);
};

#define MEASUREMENT_MOTOR_SPEED_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_MOTOR_SPEED)->FromUnitToBase(x)
#define MEASUREMENT_MOTOR_SPEED_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_MOTOR_SPEED)->FromBaseToUnit(x)
#define MEASUREMENT_MOTOR_SPEED_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_MOTOR_SPEED)->CurrentPrecision
#define MEASUREMENT_MOTOR_SPEED_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_MOTOR_SPEED_BASE_TO_UNIT(x), 'f', MEASUREMENT_MOTOR_SPEED_PRECISION())

#define MEASUREMENT_MOTOR_SPEED_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_MOTOR_SPEED)->CurrentUnitLabel
#define MEASUREMENT_MOTOR_SPEED_DISPATCHER MEASUREMENT_DISPATCHER(MEASUREMENT_MOTOR_SPEED)
#define ATTACH_MOTOR_SPEED_MEASUREMENT(delegate, min, max, step) \
    ATTACH_MEASUREMENT(MEASUREMENT_MOTOR_SPEED, delegate, min, max, step)

#endif // MOTORSPEEDDECLARATIONS_H
