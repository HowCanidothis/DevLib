#ifndef WEIGHTPERLENGTHDECLARATIONS_H
#define WEIGHTPERLENGTHDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"
static const Name MEASUREMENT_WEIGHT_PER_LENGTH = "Weight per Length";

namespace WeightPerLengthUnits
{
    static const MeasurementUnit KilogramPerMeter("KilogramPerMeter", []{return QObject::tr("kilogram per meter");}, []{ return QObject::tr("kg/m"); }, 1.0);
    static const MeasurementUnit KilogramPerCantimeter("KilogramPerCantimeter", []{return QObject::tr("kilogram per cantimeter");}, []{ return QObject::tr("kg/cm"); }, 100.0);
    static const MeasurementUnit PoundPerFoot("PoundPerFoot", []{return QObject::tr("pound per foot");}, []{ return QObject::tr("lb/ft"); }, 1.49);
    static const MeasurementUnit PoundPerInch("PoundPerInch", []{return QObject::tr("pound per inch");}, []{ return QObject::tr("lb/in"); }, 17.88);
};

#define MEASUREMENT_WEIGHT_PER_LENGTH_UNIT_TO_BASE(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_WEIGHT_PER_LENGTH)->FromUnitToBase(x)
#define MEASUREMENT_WEIGHT_PER_LENGTH_BASE_TO_UNIT(x) \
    MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_WEIGHT_PER_LENGTH)->FromBaseToUnit(x)
#define MEASUREMENT_WEIGHT_PER_LENGTH_PRECISION() \
    MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_WEIGHT_PER_LENGTH)->CurrentPrecision
#define MEASUREMENT_WEIGHT_PER_LENGTH_BASE_TO_UNIT_UI(x) \
    QString::number(MEASUREMENT_WEIGHT_PER_LENGTH_BASE_TO_UNIT(x), 'f', MEASUREMENT_WEIGHT_PER_LENGTH_PRECISION())

#define MEASUREMENT_WEIGHT_PER_LENGTH_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_WEIGHT_PER_LENGTH)->CurrentUnitLabel
#define ATTACH_WEIGHT_PER_LENGTH_MEASUREMENT(delegate, min, max) \
    ATTACH_MEASUREMENT(MEASUREMENT_WEIGHT_PER_LENGTH, delegate, min, max)

#endif // WEIGHTPERLENGTHDECLARATIONS_H
