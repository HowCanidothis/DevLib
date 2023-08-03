#include "densitydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace DensityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogramsPerCubicMeters, []{return QObject::tr("kilograms/cubic meters");}, []{ return QObject::tr("kg/m3"); }, 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(KilogramsPerLiter, []{return QObject::tr("kilograms/liter");}, []{ return QObject::tr("kg/l"); }, 1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerGallon, []{return QObject::tr("pounds/gallon");}, []{ return QObject::tr("ppg"); }, 119.8264273167)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerCubicFeet, []{return QObject::tr("pounds/cubic feet");}, []{ return QObject::tr("lb/ft3"); }, 16.01846337395)
    IMPLEMENT_MEASUREMENT_UNIT(MilligrammPerLiter, []{return QObject::tr("milligrams/liter");}, []{ return QObject::tr("mg/L"); }, 0.001)
    IMPLEMENT_MEASUREMENT_UNIT(SpecificGravity, []{return QObject::tr("specific gravity");}, []{ return QObject::tr("sg"); }, 1000.0)
}

IMPLEMENT_MEASUREMENT(Density, TR(MeasurementTr::tr("Density")))
IMPLEMENT_MEASUREMENT(SolidDensity, TR(MeasurementTr::tr("Solid Density")))
IMPLEMENT_MEASUREMENT(MudWeight, TR(MeasurementTr::tr("Mud Weight")))
