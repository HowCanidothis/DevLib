#include "pressuredeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace PressureUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Pascals,                     TR(MeasurementTr::tr("pascals")),                       TR(MeasurementTr::tr("Pa")),        1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Kilopascals,                 TR(MeasurementTr::tr("kilopascals")),                   TR(MeasurementTr::tr("kPa")),       1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(NewtonPerSquareMeter,        TR(MeasurementTr::tr("newton/sq.meter")),               TR(MeasurementTr::tr("N/m2")),      1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Bars,                        TR(MeasurementTr::tr("bars")),                          TR(MeasurementTr::tr("bars")),      100000.0)
    IMPLEMENT_MEASUREMENT_UNIT(Megapascals,                 TR(MeasurementTr::tr("megapascals")),                   TR(MeasurementTr::tr("MPa")),       1000000.0)
    IMPLEMENT_MEASUREMENT_UNIT(Atmospheres,                 TR(MeasurementTr::tr("atmospheres")),                   TR(MeasurementTr::tr("atm")),       101325.0)
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerSquareCentimeter, TR(MeasurementTr::tr("kilogram-forces/sq.centimeter")), TR(MeasurementTr::tr("kgf/cm2")),   98066.5)
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerSquareMeter,      TR(MeasurementTr::tr("kilogram-forces/sq.meter")),      TR(MeasurementTr::tr("kgf/m2")),    9.80665)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareInch,         TR(MeasurementTr::tr("pounds-force/sq.inch")),          TR(MeasurementTr::tr("psi")),       6894.757)
    IMPLEMENT_MEASUREMENT_UNIT(KilopoundsPerSquareInch,     TR(MeasurementTr::tr("kilopounds-force/sq.inch")),      TR(MeasurementTr::tr("ksi")),       6894757)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareFeet,         TR(MeasurementTr::tr("pounds-force/sq.feet")),          TR(MeasurementTr::tr("lbf/ft2")),   47.880172)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPer100SquareFeet,      TR(MeasurementTr::tr("pounds-force/100 sq.feet")),      TR(MeasurementTr::tr("lbf/100ft2")),0.47880172)
}

IMPLEMENT_MEASUREMENT(Pressure, TR(MeasurementTr::tr("Pressure")))
IMPLEMENT_MEASUREMENT(MudPressure, TR(MeasurementTr::tr("Mud Pressure")))
IMPLEMENT_MEASUREMENT(YieldPoint, TR(MeasurementTr::tr("Yield Point")))
IMPLEMENT_MEASUREMENT(YieldStrength, TR(MeasurementTr::tr("Yield Strength")))
IMPLEMENT_MEASUREMENT(YoungModulus, TR(MeasurementTr::tr("Young Modulus")))
