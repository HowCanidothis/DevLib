#include "weightperlengthdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace WeightPerLengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerMeter,        TR(MeasurementTr::tr("kilogram per meter")),        TR(MeasurementTr::tr("kg/m")),  1.0)
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerCantimeter,   TR(MeasurementTr::tr("kilogram per cantimeter")),   TR(MeasurementTr::tr("kg/cm")), 100.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerFoot,            TR(MeasurementTr::tr("pound per foot")),            TR(MeasurementTr::tr("lb/ft")), 1.48816)
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerInch,            TR(MeasurementTr::tr("pound per inch")),            TR(MeasurementTr::tr("lb/in")), 17.88)
}

IMPLEMENT_MEASUREMENT(WeightPerLength, TR(MeasurementTr::tr("Weight Per Length")))
