#include "motorspeeddeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace MotorSpeedUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerCubicMeter, TR(MeasurementTr::tr("revolution per cubic meter")),TR(MeasurementTr::tr("rev/m3")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerGallon,     TR(MeasurementTr::tr("revolution per gallon")),     TR(MeasurementTr::tr("rev/gal")), 264.172052)
    IMPLEMENT_MEASUREMENT_UNIT(RevolutionPerLiter,      TR(MeasurementTr::tr("revolution per liter")),      TR(MeasurementTr::tr("rev/l")), 1000.0)
}

IMPLEMENT_MEASUREMENT(MotorSpeed, TR(MeasurementTr::tr("Motor Speed")))
