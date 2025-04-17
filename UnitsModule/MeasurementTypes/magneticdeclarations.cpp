#include "magneticdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FieldStrengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Tesla,      TR(MeasurementTr::tr("tesla")),          TR(MeasurementTr::tr("T")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(NanoTeslas,  TR(MeasurementTr::tr("nanoTeslas")),    TR(MeasurementTr::tr("nT")),   1.e-9)
    IMPLEMENT_MEASUREMENT_UNIT(MicroTeslas, TR(MeasurementTr::tr("microTeslas")),   TR(MeasurementTr::tr("μT")),   1.e-6)
    IMPLEMENT_MEASUREMENT_UNIT(Gauss,       TR(MeasurementTr::tr("gauss")),         TR(MeasurementTr::tr("G")),    1.e-4)
    IMPLEMENT_MEASUREMENT_UNIT(WeberPerSquareMeter, TR(MeasurementTr::tr("weber / square meter")), TR(MeasurementTr::tr("Wb/m²")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(MaxwellPerSquareMeter, TR(MeasurementTr::tr("maxwell / square meter")), TR(MeasurementTr::tr("Mx/m²")), 1.0)
}

IMPLEMENT_MEASUREMENT(MagneticField, TR(MeasurementTr::tr("Magnetic Field")))

namespace MagneticFluxUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Weber,            TR(MeasurementTr::tr("Weber")),            TR(MeasurementTr::tr("Wb")),   1.0)
    IMPLEMENT_MEASUREMENT_UNIT(MicroWeber,       TR(MeasurementTr::tr("MicroWeber")),       TR(MeasurementTr::tr("μWb")),  1.e-6)
    IMPLEMENT_MEASUREMENT_UNIT(NanoWeber,        TR(MeasurementTr::tr("NanoWeber")),        TR(MeasurementTr::tr("nWb")),  1.e-9)
    IMPLEMENT_MEASUREMENT_UNIT(VoltSecod,        TR(MeasurementTr::tr("VoltSecod")),        TR(MeasurementTr::tr("V·s")),  1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Maxwell,          TR(MeasurementTr::tr("Maxwell")),          TR(MeasurementTr::tr("Mx")),   1.e-8)
    IMPLEMENT_MEASUREMENT_UNIT(TeslaSquareMeter, TR(MeasurementTr::tr("TeslaSquareMeter")), TR(MeasurementTr::tr("T·m²")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(GaussSquareMeter, TR(MeasurementTr::tr("GaussSquareMeter")), TR(MeasurementTr::tr("G·m²")), 1.e-4)
    IMPLEMENT_MEASUREMENT_UNIT(GaussSquareCentimeters, TR(MeasurementTr::tr("GaussSquareCentimeters")), TR(MeasurementTr::tr("G·cm²")), 1.e-8)
}
IMPLEMENT_MEASUREMENT(MagneticFlux, TR(MeasurementTr::tr("Magnetic Flux")))
