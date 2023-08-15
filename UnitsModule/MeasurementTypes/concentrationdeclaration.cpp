#include "concentrationdeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace ConcentrationUnits {
    IMPLEMENT_MEASUREMENT_UNIT(MilliliterPerMilliliter,     TR(MeasurementTr::tr("milliliter/milliliter")), TR(MeasurementTr::tr("mL/mL")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PartsPerBillion,             TR(MeasurementTr::tr("Parts Per Billion")),     TR(MeasurementTr::tr("ppb")),       0.0028530101742118)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerCubicMeters,   TR(MeasurementTr::tr("m3/m3")),                 TR(MeasurementTr::tr("m3/m3")),     1.)
    IMPLEMENT_MEASUREMENT_UNIT(KilogrammPerCubicMeters,     TR(MeasurementTr::tr("kg/m3")),                 TR(MeasurementTr::tr("kg/m3")),     0.001)
    IMPLEMENT_MEASUREMENT_UNIT(MilligrammPerLiter,          TR(MeasurementTr::tr("milligramm per liter")),  TR(MeasurementTr::tr("mg/L")),      0.000001)
    IMPLEMENT_MEASUREMENT_UNIT(GallonPerMegagallon,         TR(MeasurementTr::tr("Gallon Per Megagallon")), TR(MeasurementTr::tr("gal/Mgal")),  0.000001)
}

IMPLEMENT_MEASUREMENT(Concentration, TR(MeasurementTr::tr("Concentration")))
IMPLEMENT_MEASUREMENT(ConcentrationMud, TR(MeasurementTr::tr("ConcentrationMud")))
IMPLEMENT_MEASUREMENT(ConcentrationAlkalinity, TR(MeasurementTr::tr("Concentration Alkalinity")))
