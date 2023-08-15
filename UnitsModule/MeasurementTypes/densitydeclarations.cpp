#include "densitydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace DensityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogramsPerCubicMeters, TR(MeasurementTr::tr("kilograms/cubic meters")),TR(MeasurementTr::tr("kg/m3")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(KilogramsPerLiter,       TR(MeasurementTr::tr("kilograms/liter")),       TR(MeasurementTr::tr("kg/l")),  1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerGallon,         TR(MeasurementTr::tr("pounds/gallon")),         TR(MeasurementTr::tr("ppg")),   119.8264273167)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerCubicFeet,      TR(MeasurementTr::tr("pounds/cubic feet")),     TR(MeasurementTr::tr("lb/ft3")), 16.01846337395)
    IMPLEMENT_MEASUREMENT_UNIT(MilligrammPerLiter,      TR(MeasurementTr::tr("milligrams/liter")),      TR(MeasurementTr::tr("mg/L")),  0.001)
    IMPLEMENT_MEASUREMENT_UNIT(SpecificGravity,         TR(MeasurementTr::tr("specific gravity")),      TR(MeasurementTr::tr("sg")),    1000.0)
}

IMPLEMENT_MEASUREMENT(Density, TR(MeasurementTr::tr("Density")))
IMPLEMENT_MEASUREMENT(SolidDensity, TR(MeasurementTr::tr("Solid Density")))
IMPLEMENT_MEASUREMENT(MudWeight, TR(MeasurementTr::tr("Mud Weight")))
