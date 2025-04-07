#include "magneticfieldstrengthdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace MagneticFieldStrengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(AmperePerMeter,      TR(MeasurementTr::tr("nanoTeslas")),    TR(MeasurementTr::tr("A/m")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(AmpereTurnPerMeter,  TR(MeasurementTr::tr("nanoTeslas")),    TR(MeasurementTr::tr("AT/m")),   1.0)
    IMPLEMENT_MEASUREMENT_UNIT(KiloAmperePerMeter,  TR(MeasurementTr::tr("nanoTeslas")),    TR(MeasurementTr::tr("kA/m")),   1000)
    IMPLEMENT_MEASUREMENT_UNIT(Oersted,             TR(MeasurementTr::tr("nanoTeslas")),    TR(MeasurementTr::tr("Oe")),     79.57747154594)
}

IMPLEMENT_MEASUREMENT(MagneticFieldStrength, TR(MeasurementTr::tr("MagneticField Strength")))
