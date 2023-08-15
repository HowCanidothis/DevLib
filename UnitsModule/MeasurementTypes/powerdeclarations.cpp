#include "powerdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace PowerUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Watt,            TR(MeasurementTr::tr("Watt")),              TR(MeasurementTr::tr("W")),     1.0)
    IMPLEMENT_MEASUREMENT_UNIT(JoulePerSecond,  TR(MeasurementTr::tr("Joule Per Second")),  TR(MeasurementTr::tr("J/s")),   1.0)
    IMPLEMENT_MEASUREMENT_UNIT(KiloWatt,        TR(MeasurementTr::tr("KiloWatt")),          TR(MeasurementTr::tr("kW")),    1000.0)
    IMPLEMENT_MEASUREMENT_UNIT(HoursePower,     TR(MeasurementTr::tr("Hourse Power")),      TR(MeasurementTr::tr("hp")),    745.6998715823)
}

IMPLEMENT_MEASUREMENT(Power, TR(MeasurementTr::tr("Power")))
