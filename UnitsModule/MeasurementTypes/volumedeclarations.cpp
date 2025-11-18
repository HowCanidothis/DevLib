#include "volumedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace VolumeUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CubicMeter, TR(MeasurementTr::tr("cubic meter")),    TR(MeasurementTr::tr("m3")),    1.)
    IMPLEMENT_MEASUREMENT_UNIT(CubicFoot,  TR(MeasurementTr::tr("cubic foot")),     TR(MeasurementTr::tr("ft3")),   0.028316846592)
    IMPLEMENT_MEASUREMENT_UNIT(CubicInch,  TR(MeasurementTr::tr("cubic inch")),     TR(MeasurementTr::tr("in3")),   0.000016387064)
    IMPLEMENT_MEASUREMENT_UNIT(Barrel,     TR(MeasurementTr::tr("barrel")),         TR(MeasurementTr::tr("bbl")),   0.158987294928)
    IMPLEMENT_MEASUREMENT_UNIT(Gallon,     TR(MeasurementTr::tr("gallon")),         TR(MeasurementTr::tr("gal")),   0.003785411784)
    IMPLEMENT_MEASUREMENT_UNIT(Liter,      TR(MeasurementTr::tr("liter")),          TR(MeasurementTr::tr("L")),     0.001)

    IMPLEMENT_GLOBAL(double, Precision, Gallon.FromUnitToBase(1.e-3));
};

IMPLEMENT_MEASUREMENT(Volume, TR(MeasurementTr::tr("Volume")))
