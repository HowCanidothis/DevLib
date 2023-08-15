#include "percentsdeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace PercentsUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Partial, TR(MeasurementTr::tr("partial")), TR(""), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Percents, TR(MeasurementTr::tr("percents")), TR("%"), 0.01)
}

IMPLEMENT_MEASUREMENT(Percents,TR(MeasurementTr::tr("Percents")))
