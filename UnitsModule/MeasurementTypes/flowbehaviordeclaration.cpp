#include "flowbehaviordeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FlowBehaviorUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(DimensionlessIndex, TR(MeasurementTr::tr("")), TR(MeasurementTr::tr("")), 1.0)
}

IMPLEMENT_MEASUREMENT(FlowBehavior, TR(MeasurementTr::tr("Flow Behavior")))
