#include "massdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace MassUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Kilograms,   TR(MeasurementTr::tr("kilograms")), TR(MeasurementTr::tr("kg")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Grams,       TR(MeasurementTr::tr("grams")),     TR(MeasurementTr::tr("g")),     0.001)
    IMPLEMENT_MEASUREMENT_UNIT(Tonnes,      TR(MeasurementTr::tr("tonnes")),    TR(MeasurementTr::tr("t")),     1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(Pounds,      TR(MeasurementTr::tr("pounds")),    TR(MeasurementTr::tr("lbs")),   0.453592)
    IMPLEMENT_MEASUREMENT_UNIT(Kilopounds,  TR(MeasurementTr::tr("kilopounds")),TR(MeasurementTr::tr("klbs")),  453.592)
    IMPLEMENT_MEASUREMENT_UNIT(SaksCement,  TR(MeasurementTr::tr("94lb saks")), TR(MeasurementTr::tr("saks")),  42.63768278)
}

IMPLEMENT_MEASUREMENT(Mass, TR(MeasurementTr::tr("Mass")))
