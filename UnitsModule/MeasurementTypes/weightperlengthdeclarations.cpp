#include "weightperlengthdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace WeightPerLengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerMeter,TR(MeasurementTr::tr("kilogram per meter")),    TR(MeasurementTr::tr("kg/m")),  1.0)
    IMPLEMENT_MEASUREMENT_UNIT(NewtonPerMeter,  TR(MeasurementTr::tr("newton per meter")),      TR(MeasurementTr::tr("H/m")),   1.0 / GRAVITY)
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerFoot,    TR(MeasurementTr::tr("pound per foot")),        TR(MeasurementTr::tr("lb/ft")), 0.453592 * METERS_TO_FEETS_MULTIPLIER)
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerInch,    TR(MeasurementTr::tr("pound per inch")),        TR(MeasurementTr::tr("lb/in")), 0.453592 * 12.0 * METERS_TO_FEETS_MULTIPLIER)
}

IMPLEMENT_MEASUREMENT(WeightPerLength, TR(MeasurementTr::tr("Weight Per Length")))
