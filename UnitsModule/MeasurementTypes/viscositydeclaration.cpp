#include "viscositydeclaration.h"
#include "UnitsModule/measurementunitmanager.h"

namespace ViscosityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(PascalsPerSecond,    TR(MeasurementTr::tr("pascal-second")),         TR(MeasurementTr::tr("Pa*s")),  1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Centipoise,          TR(MeasurementTr::tr("centipoise")),            TR(MeasurementTr::tr("cp")),    0.001)
    IMPLEMENT_MEASUREMENT_UNIT(MilliPascalSecond,   TR(MeasurementTr::tr("millipascal-second")),    TR(MeasurementTr::tr("mPa*s")), 0.001)
}

IMPLEMENT_MEASUREMENT(Viscosity, TR(MeasurementTr::tr("Viscosity")))
