#include "forcedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace ForceUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Newton,          TR(MeasurementTr::tr("newton")),            TR(MeasurementTr::tr("N")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Kilonewton,      TR(MeasurementTr::tr("kilonewton")),        TR(MeasurementTr::tr("kN")),    1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundsForce,     TR(MeasurementTr::tr("pounds-force")),      TR(MeasurementTr::tr("lbf")),   4.44822)
    IMPLEMENT_MEASUREMENT_UNIT(KiloGrammForce,  TR(MeasurementTr::tr("kilogram-force")),    TR(MeasurementTr::tr("kgf")),   9.806650029)
    IMPLEMENT_MEASUREMENT_UNIT(KiloPoundsForce, TR(MeasurementTr::tr("1000 pounds-force")), TR(MeasurementTr::tr("kip")),   4448.22)
}

IMPLEMENT_MEASUREMENT(Force, TR(MeasurementTr::tr("Force")))
