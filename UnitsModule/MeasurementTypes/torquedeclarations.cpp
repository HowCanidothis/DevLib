#include "torquedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace TorqueUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(NewtonMeters,        TR(MeasurementTr::tr("newton meters")),         TR(MeasurementTr::tr("Nm")),    1.0)
    IMPLEMENT_MEASUREMENT_UNIT(DecaNewtonMeters,    TR(MeasurementTr::tr("decanewton meters")),     TR(MeasurementTr::tr("daNm")),  10.0)
    IMPLEMENT_MEASUREMENT_UNIT(KilonewtonMeters,    TR(MeasurementTr::tr("kilonewton meters")),     TR(MeasurementTr::tr("kNm")),   1000.0 )
    IMPLEMENT_MEASUREMENT_UNIT(PoundForceFeet,      TR(MeasurementTr::tr("pound-force feet")),      TR(MeasurementTr::tr("ftlbf")), 1.355817952003)
    IMPLEMENT_MEASUREMENT_UNIT(KilopoundForceFeet,  TR(MeasurementTr::tr("kilopound-force feet")),  TR(MeasurementTr::tr("kftlbs")), 1355.817952)
    IMPLEMENT_MEASUREMENT_UNIT(PoundFoot,           TR(MeasurementTr::tr("pound-foot")),            TR(MeasurementTr::tr("lbf*ft")), 0.73756214927727)
}

IMPLEMENT_MEASUREMENT(Torque, TR(MeasurementTr::tr("Torque")))
