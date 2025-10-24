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

    IMPLEMENT_GLOBAL(double, Precision, 1e-2);

    IMPLEMENT_GLOBAL(double, Mud, PoundsPerGallon.FromUnitToBase(9.0));///1078.437846
    IMPLEMENT_GLOBAL(double, Oil, PoundsPerGallon.FromUnitToBase(6.0));///718.9585639
    IMPLEMENT_GLOBAL(double, Pore, PoundsPerGallon.FromUnitToBase(8.33));///998.1541395
    IMPLEMENT_GLOBAL(double, Water, SpecificGravity.FromUnitToBase(1.0));
    IMPLEMENT_GLOBAL(double, SeaWater, SpecificGravity.FromUnitToBase(1.025));
    IMPLEMENT_GLOBAL(double, Steel, PoundsPerCubicFeet.FromUnitToBase(490));///7849.047053
    IMPLEMENT_GLOBAL(double, Riser, PoundsPerCubicFeet.FromUnitToBase(490));
    IMPLEMENT_GLOBAL(double, Cement, SpecificGravity.FromUnitToBase(2.7));
    IMPLEMENT_GLOBAL(double, Cutting, SpecificGravity.FromUnitToBase(2.145));

    IMPLEMENT_GLOBAL(double, Min, PoundsPerGallon.FromUnitToBase(0.1));
    IMPLEMENT_GLOBAL(double, Max, PoundsPerGallon.FromUnitToBase(30.0));
}

IMPLEMENT_MEASUREMENT(Density, TR(MeasurementTr::tr("Material Density")))
IMPLEMENT_MEASUREMENT(SolidDensity, TR(MeasurementTr::tr("Density")))
IMPLEMENT_MEASUREMENT(MudWeight, TR(MeasurementTr::tr("Mud Weight")))
