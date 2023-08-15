#include "powerareadeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace PowerAreaUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(WattPerSqMeter,          TR(MeasurementTr::tr("Watt Per Square Meters")),            TR(MeasurementTr::tr("W/m2")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(HoursePowerPerSqMeter,   TR(MeasurementTr::tr("Hourse Power Per Square Meters")),    TR(MeasurementTr::tr("hp/m2")), 745.6998715823)
    IMPLEMENT_MEASUREMENT_UNIT(HoursePowerPerSqInch,    TR(MeasurementTr::tr("Hourse Power Per Square Inch")),      TR(MeasurementTr::tr("hp/in2")), 1155837.11262664)
}

IMPLEMENT_MEASUREMENT(PowerArea, TR(MeasurementTr::tr("PowerArea")))
