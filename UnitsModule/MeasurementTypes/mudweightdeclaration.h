#ifndef MUDWEIGHTDECLARATION_H
#define MUDWEIGHTDECLARATION_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_MUD_WEIGHT = "Mud Weight";

namespace MudWeightUnits
{
    static const MeasurementUnit PoundPerGallon("Pound Per Gallon", []{return QObject::tr("pounds per gallog");}, []{ return QObject::tr("ppg"); }, 1.0);
    static const MeasurementUnit KilogramPerCubicMeter("Kilogram Per Cubic Meter", []{return QObject::tr("kilogram per cubic meter");}, []{ return QObject::tr("kg/m3"); }, 0.0083);
};

#define MEASUREMENT_MUD_WEIGHT_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_MUD_WEIGHT)->FromUnitToBase(x)
#define MEASUREMENT_MUD_WEIGHT_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_MUD_WEIGHT)->FromBaseToUnit(x)
#define MEASUREMENT_MUD_WEIGHT_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_MUD_WEIGHT)->Precision
#define MEASUREMENT_MUD_WEIGHT_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_MUD_WEIGHT_BASE_TO_UNIT(x), 'f', MEASUREMENT_MUD_WEIGHT_PRECISION())

#define MEASUREMENT_MUD_WEIGHT_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_MUD_WEIGHT)->CurrentUnitLabel

#endif // MUDWEIGHTDECLARATION_H
