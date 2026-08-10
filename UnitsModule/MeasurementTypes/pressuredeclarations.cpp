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
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareFeet,         TR(MeasurementTr::tr("pounds-force/sq.feet")),          TR(MeasurementTr::tr("lbf/ft2")),   47.8802589804)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPer100SquareFeet,      TR(MeasurementTr::tr("pounds-force/100 sq.feet")),      TR(MeasurementTr::tr("lbf/100ft2")),0.478802589804)
}

IMPLEMENT_MEASUREMENT(Pressure, TR(MeasurementTr::tr("Pressure")))
IMPLEMENT_MEASUREMENT(MudPressure, TR(MeasurementTr::tr("Mud Pressure")))
IMPLEMENT_MEASUREMENT(YieldPoint, TR(MeasurementTr::tr("Yield Point")))
IMPLEMENT_MEASUREMENT(YieldStrength, TR(MeasurementTr::tr("Yield Strength")))
IMPLEMENT_MEASUREMENT(YoungModulus, TR(MeasurementTr::tr("Young Modulus")))

namespace PressurePerDistanceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(PascalsPerMeter,              TR(MeasurementTr::tr("pascals per meter")),                 TR(MeasurementTr::tr("Pa/m")),      1.0)
    IMPLEMENT_MEASUREMENT_UNIT(KilopascalsPerMeter,          TR(MeasurementTr::tr("kilopascals per meter")),             TR(MeasurementTr::tr("kPa/m")),     1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(BarsPerMeter,                 TR(MeasurementTr::tr("bars per meter")),                    TR(MeasurementTr::tr("bars/m")),    100000.0)
    IMPLEMENT_MEASUREMENT_UNIT(MegapascalsPerMeter,          TR(MeasurementTr::tr("megapascals per meter")),             TR(MeasurementTr::tr("MPa/m")),     1000000.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareInchPerFeet,   TR(MeasurementTr::tr("pounds-force/sq.inch per feet")),     TR(MeasurementTr::tr("psi/ft")),    22620.593832)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareInchPer100Feet,TR(MeasurementTr::tr("pounds-force/sq.inch per 100 feet")), TR(MeasurementTr::tr("psi/100ft")),  226.205938)
    IMPLEMENT_MEASUREMENT_UNIT(KilopoundsPerSquareInchPerFeet, TR(MeasurementTr::tr("kilopounds-force/sq.inch per feet")), TR(MeasurementTr::tr("ksi/ft")),   22620593.832)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareInchPerUSFeet,   TR(MeasurementTr::tr("pounds-force/sq.inch per US feet")),  TR(MeasurementTr::tr("psi/usft")),      22620.548598)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareInchPer100USFeet,TR(MeasurementTr::tr("pounds-force/sq.inch per 100 US feet")),TR(MeasurementTr::tr("psi/100usft")),  226.205486)
    IMPLEMENT_MEASUREMENT_UNIT(KilopoundsPerSquareInchPerUSFeet, TR(MeasurementTr::tr("kilopounds-force/sq.inch per US feet")),TR(MeasurementTr::tr("ksi/usft")),     22620548.598)
}

IMPLEMENT_MEASUREMENT(PressurePerDistance, TR(MeasurementTr::tr("Pressure Gradient")))
