#include "funnelviscositydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FunnelViscosityUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(SecondsForCubicMeter,    TR(MeasurementTr::tr("seconds/cubic meter")),   TR(MeasurementTr::tr("sec/m3")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(SecondsForQuart,         TR(MeasurementTr::tr("seconds/quart")),         TR(MeasurementTr::tr("sec/qt")), 1.0 / 0.000946353)
}

IMPLEMENT_MEASUREMENT(FunnelViscosity, TR(MeasurementTr::tr("Funnel Viscosity")))
