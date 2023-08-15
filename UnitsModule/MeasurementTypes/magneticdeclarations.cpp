#include "magneticdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FieldStrengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(NanoTeslas,  TR(MeasurementTr::tr("nanoTeslas")),    TR(MeasurementTr::tr("nT")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(MicroTeslas, TR(MeasurementTr::tr("microTeslas")),   TR(MeasurementTr::tr("muT")),   1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(Gauss,       TR(MeasurementTr::tr("gauss")),         TR(MeasurementTr::tr("G")),     100000.0)
}

IMPLEMENT_MEASUREMENT(MagneticField, TR(MeasurementTr::tr("Magnetic Field")))
