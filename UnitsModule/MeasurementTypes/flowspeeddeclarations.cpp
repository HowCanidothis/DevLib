#include "flowspeeddeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FlowSpeedUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerSecond,    TR(MeasurementTr::tr("cubic meters per second")),   TR(MeasurementTr::tr("m3/s")),      1.0)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerMinute,    TR(MeasurementTr::tr("cubic meters per minute")),   TR(MeasurementTr::tr("m3/min")),    1.0/60)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerHour,      TR(MeasurementTr::tr("cubic meters per hour")),     TR(MeasurementTr::tr("m3/hr")),     1.0/3600)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerDay,       TR(MeasurementTr::tr("cubic meters per day")),      TR(MeasurementTr::tr("m3/day")),    1.0/3600/24)
    IMPLEMENT_MEASUREMENT_UNIT(LitersPerSecond,         TR(MeasurementTr::tr("liters per second")),         TR(MeasurementTr::tr("l/s")),       0.001)
    IMPLEMENT_MEASUREMENT_UNIT(LitersPerMinute,         TR(MeasurementTr::tr("liters per minute")),         TR(MeasurementTr::tr("l/min")),     0.00001667)
    IMPLEMENT_MEASUREMENT_UNIT(GallonsPerMinute,        TR(MeasurementTr::tr("gallons per minute")),        TR(MeasurementTr::tr("gpm")),       0.0000630902)
    IMPLEMENT_MEASUREMENT_UNIT(BarrelsPerMinute,        TR(MeasurementTr::tr("barrels per minute")),        TR(MeasurementTr::tr("bbl/min")),   0.00264979)
    IMPLEMENT_MEASUREMENT_UNIT(CubicFeetPerSecond,      TR(MeasurementTr::tr("Cubic feet per second")),     TR(MeasurementTr::tr("ft³/s")),     0.0283168466)
    IMPLEMENT_MEASUREMENT_UNIT(MilliliterPer30Min,      TR(MeasurementTr::tr("milliliter per 30 min")),     TR(MeasurementTr::tr("mL/30min")),  5.55555556e-10)

    IMPLEMENT_GLOBAL(double, Precision, GallonsPerMinute.FromUnitToBase(1.e-3));
}

IMPLEMENT_MEASUREMENT(FlowSpeed, TR(MeasurementTr::tr("Flow Speed")))
IMPLEMENT_MEASUREMENT(Filtrate, TR(MeasurementTr::tr("Filtrate")))
